/*
 * 09_graph_scc_bridges.cpp
 *
 * Advanced graph algorithms built on a single depth-first search (DFS) idea:
 *   1. Tarjan's strongly connected components (SCC)
 *   2. Bridges (cut edges) and articulation points (cut vertices)
 *   3. Topological sort (Kahn's algorithm and DFS based)
 *   4. Condensation graph (DAG of SCCs)
 *   5. Biconnected components using an edge stack
 *   6. 2-SAT solver built on SCC
 *
 * Every algorithm runs in O(V + E) time.
 *
 * Compile:  g++ -std=c++17 -Wall -Wextra -O2 09_graph_scc_bridges.cpp -o scc
 * Run:      ./scc
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Basic directed / undirected graph with adjacency lists.
// Edges store an id so bridges can be reported without ambiguity.
// ---------------------------------------------------------------------------
struct Edge {
    int to;
    int id;
};

class Graph {
public:
    explicit Graph(int n = 0, bool directed = true)
        : n_(n), directed_(directed), adj_(n), edgeCount_(0) {}

    int vertexCount() const { return n_; }
    int edgeCount() const { return edgeCount_; }
    bool isDirected() const { return directed_; }

    // Adds an edge and returns its id.
    int addEdge(int u, int v) {
        assert(u >= 0 && u < n_ && v >= 0 && v < n_);
        int id = edgeCount_++;
        adj_[u].push_back({v, id});
        if (!directed_) {
            adj_[v].push_back({u, id});
        }
        endpoints_.push_back({u, v});
        return id;
    }

    const vector<Edge>& neighbors(int u) const { return adj_[u]; }

    pair<int, int> endpoints(int edgeId) const { return endpoints_[edgeId]; }

    // Returns a new graph with all edges reversed. Used by Kosaraju-style checks.
    Graph reversed() const {
        Graph r(n_, true);
        for (int e = 0; e < edgeCount_; ++e) {
            r.addEdge(endpoints_[e].second, endpoints_[e].first);
        }
        return r;
    }

private:
    int n_;
    bool directed_;
    vector<vector<Edge>> adj_;
    vector<pair<int, int>> endpoints_;
    int edgeCount_;
};

// ---------------------------------------------------------------------------
// 1. Tarjan's strongly connected components
// ---------------------------------------------------------------------------
class TarjanSCC {
public:
    explicit TarjanSCC(const Graph& g)
        : g_(g),
          index_(g.vertexCount(), -1),
          low_(g.vertexCount(), 0),
          onStack_(g.vertexCount(), false),
          componentOf_(g.vertexCount(), -1),
          timer_(0),
          componentCount_(0) {
        for (int v = 0; v < g_.vertexCount(); ++v) {
            if (index_[v] == -1) {
                dfs(v);
            }
        }
    }

    // Component id for each vertex. Ids are given in reverse topological order
    // of the condensation: if there is an edge from component A to component B,
    // then id(A) > id(B).
    const vector<int>& componentOf() const { return componentOf_; }

    int componentCount() const { return componentCount_; }

    vector<vector<int>> components() const {
        vector<vector<int>> result(componentCount_);
        for (int v = 0; v < g_.vertexCount(); ++v) {
            result[componentOf_[v]].push_back(v);
        }
        return result;
    }

private:
    void dfs(int u) {
        index_[u] = low_[u] = timer_++;
        stk_.push(u);
        onStack_[u] = true;

        for (const Edge& e : g_.neighbors(u)) {
            int v = e.to;
            if (index_[v] == -1) {
                dfs(v);
                low_[u] = min(low_[u], low_[v]);
            } else if (onStack_[v]) {
                low_[u] = min(low_[u], index_[v]);
            }
        }

        if (low_[u] == index_[u]) {
            while (true) {
                int w = stk_.top();
                stk_.pop();
                onStack_[w] = false;
                componentOf_[w] = componentCount_;
                if (w == u) break;
            }
            ++componentCount_;
        }
    }

    const Graph& g_;
    vector<int> index_;
    vector<int> low_;
    vector<bool> onStack_;
    vector<int> componentOf_;
    stack<int> stk_;
    int timer_;
    int componentCount_;
};

// ---------------------------------------------------------------------------
// 2. Bridges and articulation points (undirected graphs)
// ---------------------------------------------------------------------------
class CutAnalysis {
public:
    explicit CutAnalysis(const Graph& g)
        : g_(g),
          tin_(g.vertexCount(), -1),
          low_(g.vertexCount(), 0),
          isArticulation_(g.vertexCount(), false),
          timer_(0) {
        assert(!g.isDirected());
        for (int v = 0; v < g_.vertexCount(); ++v) {
            if (tin_[v] == -1) {
                dfs(v, -1);
            }
        }
    }

    const vector<int>& bridges() const { return bridges_; }

    vector<int> articulationPoints() const {
        vector<int> out;
        for (int v = 0; v < g_.vertexCount(); ++v) {
            if (isArticulation_[v]) out.push_back(v);
        }
        return out;
    }

private:
    // parentEdge is the id of the edge we came in by. Using the edge id
    // (not the parent vertex) lets the code handle multi-edges correctly.
    void dfs(int u, int parentEdge) {
        tin_[u] = low_[u] = timer_++;
        int children = 0;

        for (const Edge& e : g_.neighbors(u)) {
            if (e.id == parentEdge) continue;
            int v = e.to;
            if (tin_[v] != -1) {
                low_[u] = min(low_[u], tin_[v]);
            } else {
                ++children;
                dfs(v, e.id);
                low_[u] = min(low_[u], low_[v]);

                if (low_[v] > tin_[u]) {
                    bridges_.push_back(e.id);
                }
                if (parentEdge != -1 && low_[v] >= tin_[u]) {
                    isArticulation_[u] = true;
                }
            }
        }

        // The root of a DFS tree is an articulation point if it has 2+ children.
        if (parentEdge == -1 && children > 1) {
            isArticulation_[u] = true;
        }
    }

    const Graph& g_;
    vector<int> tin_;
    vector<int> low_;
    vector<bool> isArticulation_;
    vector<int> bridges_;
    int timer_;
};

// ---------------------------------------------------------------------------
// 3. Biconnected components (vertex-biconnected blocks) with an edge stack
// ---------------------------------------------------------------------------
class BiconnectedComponents {
public:
    explicit BiconnectedComponents(const Graph& g)
        : g_(g),
          tin_(g.vertexCount(), -1),
          low_(g.vertexCount(), 0),
          timer_(0) {
        assert(!g.isDirected());
        for (int v = 0; v < g_.vertexCount(); ++v) {
            if (tin_[v] == -1) {
                dfs(v, -1);
                // Any edges left on the stack belong to the last component.
                flushUntil(-1);
            }
        }
    }

    // Each block is a list of edge ids.
    const vector<vector<int>>& blocks() const { return blocks_; }

private:
    void dfs(int u, int parentEdge) {
        tin_[u] = low_[u] = timer_++;
        for (const Edge& e : g_.neighbors(u)) {
            if (e.id == parentEdge) continue;
            int v = e.to;
            if (tin_[v] == -1) {
                edgeStack_.push(e.id);
                dfs(v, e.id);
                low_[u] = min(low_[u], low_[v]);
                if (low_[v] >= tin_[u]) {
                    // Pop edges until we pop the tree edge (u, v).
                    flushUntil(e.id);
                }
            } else if (tin_[v] < tin_[u]) {
                // Back edge: push once, from the deeper endpoint.
                edgeStack_.push(e.id);
                low_[u] = min(low_[u], tin_[v]);
            }
        }
    }

    void flushUntil(int stopEdge) {
        vector<int> block;
        while (!edgeStack_.empty()) {
            int id = edgeStack_.top();
            edgeStack_.pop();
            block.push_back(id);
            if (id == stopEdge) break;
        }
        if (!block.empty()) {
            blocks_.push_back(block);
        }
    }

    const Graph& g_;
    vector<int> tin_;
    vector<int> low_;
    stack<int> edgeStack_;
    vector<vector<int>> blocks_;
    int timer_;
};

// ---------------------------------------------------------------------------
// 4. Topological sort: Kahn's algorithm (BFS) and DFS based
// ---------------------------------------------------------------------------
namespace topo {

// Returns an empty vector if the graph has a cycle.
vector<int> kahn(const Graph& g) {
    int n = g.vertexCount();
    vector<int> indeg(n, 0);
    for (int u = 0; u < n; ++u) {
        for (const Edge& e : g.neighbors(u)) {
            indeg[e.to]++;
        }
    }
    queue<int> q;
    for (int v = 0; v < n; ++v) {
        if (indeg[v] == 0) q.push(v);
    }
    vector<int> order;
    order.reserve(n);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (const Edge& e : g.neighbors(u)) {
            if (--indeg[e.to] == 0) q.push(e.to);
        }
    }
    if (static_cast<int>(order.size()) != n) {
        return {};
    }
    return order;
}

namespace detail {
// Three-colour DFS: 0 = white, 1 = grey (on stack), 2 = black (done).
bool dfsOrder(const Graph& g, int u, vector<int>& colour, vector<int>& out) {
    colour[u] = 1;
    for (const Edge& e : g.neighbors(u)) {
        if (colour[e.to] == 1) return false;  // back edge -> cycle
        if (colour[e.to] == 0 && !dfsOrder(g, e.to, colour, out)) return false;
    }
    colour[u] = 2;
    out.push_back(u);
    return true;
}
}  // namespace detail

// Returns an empty vector if the graph has a cycle.
vector<int> dfsOrder(const Graph& g) {
    int n = g.vertexCount();
    vector<int> colour(n, 0);
    vector<int> out;
    out.reserve(n);
    for (int v = 0; v < n; ++v) {
        if (colour[v] == 0 && !detail::dfsOrder(g, v, colour, out)) {
            return {};
        }
    }
    reverse(out.begin(), out.end());
    return out;
}

// Lexicographically smallest topological order (uses a min-heap in Kahn).
vector<int> lexSmallest(const Graph& g) {
    int n = g.vertexCount();
    vector<int> indeg(n, 0);
    for (int u = 0; u < n; ++u) {
        for (const Edge& e : g.neighbors(u)) indeg[e.to]++;
    }
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int v = 0; v < n; ++v) {
        if (indeg[v] == 0) pq.push(v);
    }
    vector<int> order;
    while (!pq.empty()) {
        int u = pq.top();
        pq.pop();
        order.push_back(u);
        for (const Edge& e : g.neighbors(u)) {
            if (--indeg[e.to] == 0) pq.push(e.to);
        }
    }
    if (static_cast<int>(order.size()) != n) return {};
    return order;
}

}  // namespace topo

// ---------------------------------------------------------------------------
// 5. Condensation: build the DAG of SCCs
// ---------------------------------------------------------------------------
struct Condensation {
    Graph dag;                  // one vertex per SCC
    vector<int> componentOf;    // original vertex -> SCC id
};

Condensation buildCondensation(const Graph& g) {
    TarjanSCC scc(g);
    int k = scc.componentCount();
    Graph dag(k, true);
    set<pair<int, int>> seen;  // remove parallel edges between the same pair
    for (int e = 0; e < g.edgeCount(); ++e) {
        auto [u, v] = g.endpoints(e);
        int cu = scc.componentOf()[u];
        int cv = scc.componentOf()[v];
        if (cu != cv && seen.insert({cu, cv}).second) {
            dag.addEdge(cu, cv);
        }
    }
    return {dag, scc.componentOf()};
}

// ---------------------------------------------------------------------------
// 6. 2-SAT solver built on SCC
//    Variables: 0..n-1. Literal x means "variable x is true",
//    literal x+n means "variable x is false". Negation of lit l is l ^ (l >= n).
// ---------------------------------------------------------------------------
class TwoSat {
public:
    explicit TwoSat(int variables)
        : n_(variables), g_(2 * variables, true) {}

    // Adds clause (a OR b). Literals are given as signed values:
    // +x means x is true, -x means x is false, x in [1, n].
    void addClause(int a, int b) {
        int la = toNode(a);
        int lb = toNode(b);
        // (a or b) == (!a -> b) and (!b -> a)
        g_.addEdge(la ^ 1, lb);
        g_.addEdge(lb ^ 1, la);
    }

    // Returns true and fills assignment (1-based) if satisfiable.
    bool solve(vector<bool>& assignment) {
        TarjanSCC scc(g_);
        const vector<int>& comp = scc.componentOf();
        assignment.assign(n_ + 1, false);
        for (int x = 0; x < n_; ++x) {
            if (comp[2 * x] == comp[2 * x + 1]) return false;
            // Tarjan ids are in reverse topological order, so pick the literal
            // whose component comes later in topological order (smaller id).
            assignment[x + 1] = comp[2 * x] < comp[2 * x + 1];
        }
        return true;
    }

private:
    // Map signed literal (1..n or -1..-n) to a node index.
    int toNode(int lit) const {
        assert(lit != 0 && lit >= -n_ && lit <= n_);
        if (lit > 0) return 2 * (lit - 1);
        return 2 * (-lit - 1) + 1;
    }

    int n_;
    Graph g_;
};

// ---------------------------------------------------------------------------
// Helpers for printing
// ---------------------------------------------------------------------------
void printComponents(const string& title, const vector<vector<int>>& comps) {
    cout << title << " (" << comps.size() << " components)\n";
    for (size_t i = 0; i < comps.size(); ++i) {
        cout << "  C" << i << ": { ";
        for (size_t j = 0; j < comps[i].size(); ++j) {
            cout << comps[i][j] << (j + 1 < comps[i].size() ? ", " : " ");
        }
        cout << "}\n";
    }
}

void printVector(const string& title, const vector<int>& v) {
    cout << title << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << v[i] << (i + 1 < v.size() ? ", " : "");
    }
    cout << "]\n";
}

// ---------------------------------------------------------------------------
// Brute force reference implementations used in tests.
// ---------------------------------------------------------------------------

// Reachability matrix via BFS from every vertex. O(V * (V + E)).
vector<vector<bool>> reachability(const Graph& g) {
    int n = g.vertexCount();
    vector<vector<bool>> r(n, vector<bool>(n, false));
    for (int s = 0; s < n; ++s) {
        queue<int> q;
        q.push(s);
        r[s][s] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const Edge& e : g.neighbors(u)) {
                if (!r[s][e.to]) {
                    r[s][e.to] = true;
                    q.push(e.to);
                }
            }
        }
    }
    return r;
}

// Two vertices are in the same SCC iff they reach each other.
bool sameSccBruteForce(const vector<vector<bool>>& r, int a, int b) {
    return r[a][b] && r[b][a];
}

// Number of connected components of an undirected graph, ignoring edge ids.
int connectedComponentsCount(const Graph& g) {
    int n = g.vertexCount();
    vector<bool> seen(n, false);
    int count = 0;
    for (int s = 0; s < n; ++s) {
        if (seen[s]) continue;
        ++count;
        queue<int> q;
        q.push(s);
        seen[s] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const Edge& e : g.neighbors(u)) {
                if (!seen[e.to]) {
                    seen[e.to] = true;
                    q.push(e.to);
                }
            }
        }
    }
    return count;
}

// Brute force bridge check: remove an edge and see if connectivity increases.
vector<int> bridgesBruteForce(const Graph& g) {
    vector<int> result;
    int base = connectedComponentsCount(g);
    for (int e = 0; e < g.edgeCount(); ++e) {
        Graph h(g.vertexCount(), false);
        for (int f = 0; f < g.edgeCount(); ++f) {
            if (f == e) continue;
            auto [u, v] = g.endpoints(f);
            h.addEdge(u, v);
        }
        if (connectedComponentsCount(h) > base) {
            result.push_back(e);
        }
    }
    return result;
}

// Brute force articulation points: remove vertex v and count components.
vector<int> articulationBruteForce(const Graph& g) {
    vector<int> result;
    int n = g.vertexCount();
    int base = connectedComponentsCount(g);
    for (int v = 0; v < n; ++v) {
        Graph h(n - 1, false);
        auto mapId = [v](int x) { return x < v ? x : x - 1; };
        for (int e = 0; e < g.edgeCount(); ++e) {
            auto [a, b] = g.endpoints(e);
            if (a == v || b == v) continue;
            h.addEdge(mapId(a), mapId(b));
        }
        // Removing v also removes its own component if it was isolated, so
        // compare against the original count minus one when v was alone.
        int after = connectedComponentsCount(h);
        bool wasIsolated = g.neighbors(v).empty();
        if (!wasIsolated && after > base) {
            result.push_back(v);
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Test cases
// ---------------------------------------------------------------------------
int testsRun = 0;
int testsPassed = 0;

void check(bool cond, const string& name) {
    ++testsRun;
    if (cond) {
        ++testsPassed;
        cout << "  [PASS] " << name << "\n";
    } else {
        cout << "  [FAIL] " << name << "\n";
    }
}

void testTarjanSmall() {
    cout << "\n== Tarjan SCC: small example ==\n";
    // 0 -> 1 -> 2 -> 0 forms one cycle, 3 -> 4 -> 3 forms another, 5 alone.
    Graph g(6, true);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 3);
    TarjanSCC scc(g);
    printComponents("SCCs", scc.components());
    check(scc.componentCount() == 3, "three SCCs found");
    check(scc.componentOf()[0] == scc.componentOf()[1], "0 and 1 share an SCC");
    check(scc.componentOf()[1] == scc.componentOf()[2], "1 and 2 share an SCC");
    check(scc.componentOf()[3] == scc.componentOf()[4], "3 and 4 share an SCC");
    check(scc.componentOf()[2] != scc.componentOf()[3], "cycles are separate");
    check(scc.componentOf()[5] != scc.componentOf()[0], "isolated vertex is alone");
}

void testTarjanRandomAgainstBruteForce() {
    cout << "\n== Tarjan SCC: random graphs vs brute force ==\n";
    unsigned seed = 12345;
    auto rnd = [&seed](int mod) {
        seed = seed * 1103515245u + 12345u;
        return static_cast<int>((seed >> 16) % static_cast<unsigned>(mod));
    };
    bool allOk = true;
    for (int trial = 0; trial < 50; ++trial) {
        int n = 3 + rnd(10);
        int m = rnd(3 * n);
        Graph g(n, true);
        for (int i = 0; i < m; ++i) {
            g.addEdge(rnd(n), rnd(n));
        }
        TarjanSCC scc(g);
        auto r = reachability(g);
        for (int a = 0; a < n && allOk; ++a) {
            for (int b = 0; b < n && allOk; ++b) {
                bool expect = sameSccBruteForce(r, a, b);
                bool got = scc.componentOf()[a] == scc.componentOf()[b];
                if (expect != got) allOk = false;
            }
        }
    }
    check(allOk, "50 random digraphs match brute force SCC relation");
}

void testCondensationIsDag() {
    cout << "\n== Condensation graph ==\n";
    Graph g(5, true);
    g.addEdge(0, 1);
    g.addEdge(1, 0);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 2);
    g.addEdge(3, 4);
    Condensation c = buildCondensation(g);
    printVector("component of each vertex", c.componentOf);
    check(c.dag.vertexCount() == 3, "condensation has 3 nodes");
    check(!topo::kahn(c.dag).empty(), "condensation is acyclic");
}

void testCutsSmall() {
    cout << "\n== Bridges and articulation points: small example ==\n";
    // Two triangles joined by one bridge edge (2-3), plus pendant vertex 6 on 5.
    Graph g(7, false);
    int e01 = g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    int bridge = g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 3);
    int pendant = g.addEdge(5, 6);
    (void)e01;

    CutAnalysis cuts(g);
    vector<int> br = cuts.bridges();
    sort(br.begin(), br.end());
    printVector("bridge edge ids", br);
    vector<int> ap = cuts.articulationPoints();
    printVector("articulation points", ap);

    check(find(br.begin(), br.end(), bridge) != br.end(), "edge 2-3 is a bridge");
    check(find(br.begin(), br.end(), pendant) != br.end(), "pendant edge 5-6 is a bridge");
    check(find(ap.begin(), ap.end(), 2) != ap.end(), "vertex 2 is an articulation point");
    check(find(ap.begin(), ap.end(), 3) != ap.end(), "vertex 3 is an articulation point");
    check(find(ap.begin(), ap.end(), 5) != ap.end(), "vertex 5 is an articulation point");
}

void testCutsRandomAgainstBruteForce() {
    cout << "\n== Bridges and articulation points: random vs brute force ==\n";
    unsigned seed = 999;
    auto rnd = [&seed](int mod) {
        seed = seed * 1103515245u + 12345u;
        return static_cast<int>((seed >> 16) % static_cast<unsigned>(mod));
    };
    bool bridgeOk = true;
    bool apOk = true;
    for (int trial = 0; trial < 40; ++trial) {
        int n = 3 + rnd(8);
        int m = n + rnd(2 * n);
        Graph g(n, false);
        for (int i = 0; i < m; ++i) {
            int u = rnd(n), v = rnd(n);
            if (u != v) g.addEdge(u, v);
        }
        CutAnalysis cuts(g);
        vector<int> expectedBridges = bridgesBruteForce(g);
        vector<int> gotBridges = cuts.bridges();
        sort(expectedBridges.begin(), expectedBridges.end());
        sort(gotBridges.begin(), gotBridges.end());
        if (expectedBridges != gotBridges) bridgeOk = false;

        vector<int> expectedAp = articulationBruteForce(g);
        vector<int> gotAp = cuts.articulationPoints();
        sort(expectedAp.begin(), expectedAp.end());
        if (expectedAp != gotAp) apOk = false;
    }
    check(bridgeOk, "bridges match brute force on 40 random graphs");
    check(apOk, "articulation points match brute force on 40 random graphs");
}

void testBiconnected() {
    cout << "\n== Biconnected components ==\n";
    // Triangle 0-1-2 and a second triangle 2-3-4 share vertex 2.
    Graph g(5, false);
    for (int i = 0; i < 3; ++i) {}
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 2);
    BiconnectedComponents bcc(g);
    cout << "  blocks found: " << bcc.blocks().size() << "\n";
    for (size_t i = 0; i < bcc.blocks().size(); ++i) {
        printVector("  block " + to_string(i) + " edges", bcc.blocks()[i]);
    }
    check(bcc.blocks().size() == 2, "two biconnected blocks");
    bool sizesOk = true;
    for (const auto& b : bcc.blocks()) {
        if (b.size() != 3) sizesOk = false;
    }
    check(sizesOk, "each block has 3 edges");
}

void testTopological() {
    cout << "\n== Topological sort ==\n";
    // Classic course schedule DAG.
    Graph g(6, true);
    g.addEdge(5, 2);
    g.addEdge(5, 0);
    g.addEdge(4, 0);
    g.addEdge(4, 1);
    g.addEdge(2, 3);
    g.addEdge(3, 1);

    vector<int> k = topo::kahn(g);
    vector<int> d = topo::dfsOrder(g);
    vector<int> l = topo::lexSmallest(g);
    printVector("Kahn order", k);
    printVector("DFS order", d);
    printVector("lex smallest order", l);

    auto validOrder = [&g](const vector<int>& order) {
        if (order.size() != static_cast<size_t>(g.vertexCount())) return false;
        vector<int> pos(g.vertexCount());
        for (int i = 0; i < static_cast<int>(order.size()); ++i) pos[order[i]] = i;
        for (int u = 0; u < g.vertexCount(); ++u) {
            for (const Edge& e : g.neighbors(u)) {
                if (pos[u] >= pos[e.to]) return false;
            }
        }
        return true;
    };
    check(validOrder(k), "Kahn order respects all edges");
    check(validOrder(d), "DFS order respects all edges");
    check(validOrder(l), "lex smallest order respects all edges");
    check(l == vector<int>({4, 5, 0, 2, 3, 1}), "lex smallest order is exact");

    Graph cyc(3, true);
    cyc.addEdge(0, 1);
    cyc.addEdge(1, 2);
    cyc.addEdge(2, 0);
    check(topo::kahn(cyc).empty(), "Kahn detects a cycle");
    check(topo::dfsOrder(cyc).empty(), "DFS detects a cycle");
}

void testTwoSat() {
    cout << "\n== 2-SAT ==\n";
    // (x1 or x2) and (!x1 or x2) and (!x2 or x3) and (!x3 or !x1)
    TwoSat s(3);
    s.addClause(1, 2);
    s.addClause(-1, 2);
    s.addClause(-2, 3);
    s.addClause(-3, -1);
    vector<bool> a;
    bool sat = s.solve(a);
    check(sat, "first formula is satisfiable");
    if (sat) {
        bool c1 = a[1] || a[2];
        bool c2 = !a[1] || a[2];
        bool c3 = !a[2] || a[3];
        bool c4 = !a[3] || !a[1];
        check(c1 && c2 && c3 && c4, "assignment satisfies every clause");
        cout << "  assignment: x1=" << a[1] << " x2=" << a[2] << " x3=" << a[3] << "\n";
    }

    // Unsatisfiable: x1 must be true and false.
    TwoSat u(1);
    u.addClause(1, 1);
    u.addClause(-1, -1);
    vector<bool> b;
    check(!u.solve(b), "contradiction is reported as unsatisfiable");
}

int main() {
    cout << "09 - Graph algorithms: SCC, bridges, topological sort, 2-SAT\n";

    testTarjanSmall();
    testTarjanRandomAgainstBruteForce();
    testCondensationIsDag();
    testCutsSmall();
    testCutsRandomAgainstBruteForce();
    testBiconnected();
    testTopological();
    testTwoSat();

    cout << "\nSummary: " << testsPassed << " / " << testsRun << " checks passed\n";
    return testsPassed == testsRun ? 0 : 1;
}
