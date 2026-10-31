/*
 * ============================================================================
 *  File    : 08_union_find_mst_topo.cpp
 *  Topic   : DSU, MST (Kruskal & Prim), topological sort, cycle detection
 *  Standard: C++17
 *  Build   : g++ -std=c++17 -Wall -Wextra -O2 08_union_find_mst_topo.cpp -o ufmt
 * ----------------------------------------------------------------------------
 *  Contents
 *    1. Disjoint Set Union (Union-Find)
 *         - union by rank (and by size variant) + path compression
 *         - find with iterative path halving
 *         - rollback DSU (undoable union, no path compression) for
 *           offline dynamic connectivity / divide-and-conquer problems
 *    2. Kruskal's MST algorithm  O(E log E)  (sort edges, DSU to join)
 *    3. Prim's MST algorithm with std::priority_queue  O((V+E) log V)
 *    4. Topological sort
 *         - Kahn's algorithm (BFS on in-degrees)  O(V+E)
 *         - DFS-based topological sort  O(V+E)
 *    5. Cycle detection
 *         - directed graph: Kahn (in-degree) and DFS colouring
 *         - undirected graph: DSU and DFS
 *    6. Randomized stress tests comparing every implementation against a
 *       brute-force reference (exhaustive MST by bitmask, Floyd-Warshall
 *       reachability, brute-force topological orders).
 *
 *  Complexity notes
 *    - find() with path compression + union by rank: amortized
 *      O(alpha(n)) per operation, where alpha is the inverse Ackermann
 *      function (practically constant, <= 4 for any n that fits in memory).
 *    - rollback DSU: O(log n) per union/find (no compression, rank keeps
 *      the tree height logarithmic), O(1) amortized per rollback.
 *    - Kruskal: O(E log E) dominated by sorting; the DSU part is
 *      O(E * alpha(V)).
 *    - Prim with a binary heap: O((V + E) log V).
 *    - Kahn / DFS topological sort and cycle detection: O(V + E).
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

using ll = long long;
static const ll INF = numeric_limits<ll>::max() / 4;

// ============================================================================
// Section 1: Disjoint Set Union (Union-Find)
// ============================================================================

// Classic DSU with union by rank and path compression.
// find(): O(alpha(n)) amortized; unite(): O(alpha(n)) amortized.
class DSU {
public:
    explicit DSU(int n) : parent_(n), rank_(n, 0), size_(n, 1) {
        iota(parent_.begin(), parent_.end(), 0);
    }

    // Iterative find with path compression (halving: every other node on the
    // path is re-pointed directly at its grandparent).
    int find(int x) {
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];  // path halving
            x = parent_[x];
        }
        return x;
    }

    // Union by rank: attach the shorter tree under the taller one.
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rank_[a] < rank_[b]) swap(a, b);
        parent_[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        size_[a] += size_[b];
        return true;
    }

    bool same(int a, int b) { return find(a) == find(b); }

    int size(int x) { return size_[find(x)]; }

    int components() const {
        int c = 0;
        for (int i = 0; i < (int)parent_.size(); ++i)
            if (parent_[i] == i) ++c;
        return c;
    }

private:
    vector<int> parent_, rank_, size_;
};

// Rollback DSU: supports undo() of the last unite() call. Used in offline
// dynamic connectivity (e.g. divide & conquer over time). Because we cannot
// compress paths (that would break undo), we rely on union by rank alone,
// which keeps every tree height O(log n).
class RollbackDSU {
public:
    explicit RollbackDSU(int n) : parent_(n), rank_(n, 0) {
        iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int x) const {
        while (parent_[x] != x) x = parent_[x];
        return x;
    }

    // Returns true if a and b were in different sets before this call.
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) {
            history_.push_back({-1, -1, 0});
            return false;
        }
        if (rank_[a] < rank_[b]) swap(a, b);
        history_.push_back({b, a, rank_[a]});
        parent_[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        return true;
    }

    bool same(int a, int b) { return find(a) == find(b); }

    // Undo the most recent unite() (even a no-op one).
    void undo() {
        auto [child, root, oldRank] = history_.back();
        history_.pop_back();
        if (child == -1) return;  // was a no-op
        parent_[child] = child;
        rank_[root] = oldRank;
    }

    int snapshot() const { return (int)history_.size(); }
    void rollback(int snap) {
        while ((int)history_.size() > snap) undo();
    }

private:
    vector<int> parent_, rank_;
    // (child, root, rank of root before the union)
    vector<tuple<int, int, int>> history_;
};

// ============================================================================
// Section 2: Kruskal's MST
// ============================================================================

struct Edge {
    int u, v;
    ll w;
};

// Returns the MST edges (sorted by weight) and the total weight.
// O(E log E) time, O(E) extra space.
pair<vector<Edge>, ll> kruskalMST(int n, vector<Edge> edges) {
    sort(edges.begin(), edges.end(),
         [](const Edge& a, const Edge& b) { return a.w < b.w; });
    DSU dsu(n);
    vector<Edge> mst;
    ll total = 0;
    for (const Edge& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            mst.push_back(e);
            total += e.w;
        }
    }
    return {mst, total};
}

// ============================================================================
// Section 3: Prim's MST with a priority queue
// ============================================================================

// Returns the MST edges and total weight. O((V + E) log V).
// Uses lazy deletion: stale heap entries (already in the tree) are skipped.
pair<vector<Edge>, ll> primMST(int n, const vector<vector<pair<int, ll>>>& adj) {
    vector<char> inTree(n, false);
    vector<ll> best(n, INF);
    vector<int> bestFrom(n, -1);
    priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<>> pq;
    best[0] = 0;
    pq.push({0, 0});
    vector<Edge> mst;
    ll total = 0;
    int added = 0;
    while (!pq.empty() && added < n) {
        auto [d, u] = pq.top();
        pq.pop();
        if (inTree[u] || d != best[u]) continue;  // stale entry
        inTree[u] = true;
        ++added;
        if (bestFrom[u] != -1) {
            mst.push_back({bestFrom[u], u, best[u]});
            total += best[u];
        }
        for (auto [v, w] : adj[u]) {
            if (!inTree[v] && w < best[v]) {
                best[v] = w;
                bestFrom[v] = u;
                pq.push({w, v});
            }
        }
    }
    return {mst, total};
}

// ============================================================================
// Section 4: Topological sort
// ============================================================================

// Kahn's algorithm: repeatedly remove vertices with in-degree 0.
// Returns the order, or an empty vector if the graph has a cycle.
// O(V + E).
vector<int> topoKahn(int n, const vector<vector<int>>& adj) {
    vector<int> indeg(n, 0);
    for (int u = 0; u < n; ++u)
        for (int v : adj[u]) ++indeg[v];
    queue<int> q;
    for (int u = 0; u < n; ++u)
        if (indeg[u] == 0) q.push(u);
    vector<int> order;
    order.reserve(n);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (int v : adj[u])
            if (--indeg[v] == 0) q.push(v);
    }
    if ((int)order.size() != n) order.clear();  // cycle present
    return order;
}

// DFS-based topological sort. Returns the order, or an empty vector if the
// graph has a cycle. O(V + E). The order is the reverse of the DFS finish
// times.
vector<int> topoDFS(int n, const vector<vector<int>>& adj) {
    vector<char> state(n, 0);  // 0 = unvisited, 1 = in stack, 2 = done
    vector<int> order;
    order.reserve(n);
    bool hasCycle = false;
    function<void(int)> dfs = [&](int u) {
        state[u] = 1;
        for (int v : adj[u]) {
            if (state[v] == 0) {
                dfs(v);
                if (hasCycle) return;
            } else if (state[v] == 1) {
                hasCycle = true;
                return;
            }
        }
        state[u] = 2;
        order.push_back(u);
    };
    for (int u = 0; u < n && !hasCycle; ++u)
        if (state[u] == 0) dfs(u);
    if (hasCycle) return {};
    reverse(order.begin(), order.end());
    return order;
}

// ============================================================================
// Section 5: Cycle detection
// ============================================================================

// Directed graph, Kahn-based: a cycle exists iff Kahn's algorithm cannot
// process all vertices. O(V + E).
bool hasCycleDirectedKahn(int n, const vector<vector<int>>& adj) {
    return topoKahn(n, adj).empty();
}

// Directed graph, DFS colouring (white / grey / black). O(V + E).
bool hasCycleDirectedDFS(int n, const vector<vector<int>>& adj) {
    vector<char> color(n, 0);  // 0 white, 1 grey, 2 black
    bool cycle = false;
    function<void(int)> dfs = [&](int u) {
        color[u] = 1;
        for (int v : adj[u]) {
            if (color[v] == 0) {
                dfs(v);
                if (cycle) return;
            } else if (color[v] == 1) {
                cycle = true;
                return;
            }
        }
        color[u] = 2;
    };
    for (int u = 0; u < n && !cycle; ++u)
        if (color[u] == 0) dfs(u);
    return cycle;
}

// Undirected graph, DSU-based: an edge whose endpoints are already connected
// closes a cycle. O(E * alpha(V)).
bool hasCycleUndirectedDSU(int n, const vector<pair<int, int>>& edges) {
    DSU dsu(n);
    for (auto [u, v] : edges) {
        if (dsu.same(u, v)) return true;
        dsu.unite(u, v);
    }
    return false;
}

// Undirected graph, DFS-based (parent tracking). O(V + E).
bool hasCycleUndirectedDFS(int n, const vector<vector<int>>& adj) {
    vector<char> visited(n, false);
    function<bool(int, int)> dfs = [&](int u, int parent) -> bool {
        visited[u] = true;
        for (int v : adj[u]) {
            if (v == parent) continue;
            if (visited[v]) return true;  // back edge to an ancestor
            if (dfs(v, u)) return true;
        }
        return false;
    };
    for (int u = 0; u < n; ++u)
        if (!visited[u] && dfs(u, -1)) return true;
    return false;
}

// ============================================================================
// Section 6: Brute-force references for stress testing
// ============================================================================

// Exhaustive MST by bitmask over all 2^E subsets. Only usable for tiny E.
pair<vector<Edge>, ll> bruteMST(int n, const vector<Edge>& edges) {
    int m = (int)edges.size();
    ll best = INF;
    vector<Edge> bestEdges;
    for (int mask = 0; mask < (1 << m); ++mask) {
        DSU dsu(n);
        ll cost = 0;
        int cnt = 0;
        vector<Edge> cur;
        for (int i = 0; i < m; ++i) {
            if (mask & (1 << i)) {
                if (dsu.unite(edges[i].u, edges[i].v)) {
                    cost += edges[i].w;
                    ++cnt;
                    cur.push_back(edges[i]);
                }
            }
        }
        if (cnt == n - 1 && cost < best) {
            best = cost;
            bestEdges = cur;
        }
    }
    return {bestEdges, best};
}

// Reachability by Floyd-Warshall (transitive closure). O(V^3).
vector<vector<bool>> bruteReach(int n, const vector<vector<int>>& adj) {
    vector<vector<bool>> r(n, vector<bool>(n, false));
    for (int u = 0; u < n; ++u) {
        r[u][u] = true;
        for (int v : adj[u]) r[u][v] = true;
    }
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            if (r[i][k])
                for (int j = 0; j < n; ++j)
                    if (r[k][j]) r[i][j] = true;
    return r;
}

// Brute-force cycle detection on a directed graph via reachability:
// a cycle exists iff some vertex can reach itself through a non-empty path.
bool bruteCycleDirected(int n, const vector<vector<int>>& adj) {
    auto r = bruteReach(n, adj);
    for (int u = 0; u < n; ++u)
        for (int v : adj[u])
            if (r[v][u]) return true;  // u -> v and v -> ... -> u
    return false;
}

// Brute-force cycle detection on an undirected graph: a connected component
// with E >= V contains a cycle (forest property).
bool bruteCycleUndirected(int n, const vector<pair<int, int>>& edges) {
    DSU dsu(n);
    for (auto [u, v] : edges) dsu.unite(u, v);
    vector<int> compEdge(n, 0), compVert(n, 0);
    for (auto [u, v] : edges) {
        compEdge[dsu.find(u)]++;
    }
    for (int u = 0; u < n; ++u) compVert[dsu.find(u)]++;
    for (int u = 0; u < n; ++u)
        if (compVert[u] > 0 && compEdge[u] >= compVert[u]) return true;
    return false;
}

// ============================================================================
// Section 7: Randomized stress tests
// ============================================================================

static mt19937 rng(123456789);

static int randInt(int lo, int hi) {
    return uniform_int_distribution<int>(lo, hi)(rng);
}

static ll randWeight() { return randInt(1, 100); }

// Build a random *connected* undirected weighted graph: a random spanning
// tree plus `extra` random extra edges. Guarantees an MST exists.
static vector<Edge> randomConnectedGraph(int n, int extra) {
    set<pair<int, int>> seen;
    vector<Edge> edges;
    // Random spanning tree: connect vertex i to a random earlier vertex.
    for (int i = 1; i < n; ++i) {
        int u = randInt(0, i - 1);
        seen.insert({u, i});
        edges.push_back({u, i, randWeight()});
    }
    // Extra random edges.
    while (extra-- > 0) {
        int u = randInt(0, n - 1), v = randInt(0, n - 1);
        if (u == v) continue;
        if (u > v) swap(u, v);
        if (!seen.insert({u, v}).second) continue;
        edges.push_back({u, v, randWeight()});
    }
    return edges;
}

static void testDSU() {
    cout << "[DSU] randomized stress test vs. brute-force union-find\n";
    for (int iter = 0; iter < 200; ++iter) {
        int n = randInt(1, 60);
        DSU dsu(n);
        // Reference: plain parent array without any optimizations.
        vector<int> ref(n);
        iota(ref.begin(), ref.end(), 0);
        auto refFind = [&](int x) {
            while (ref[x] != x) x = ref[x];
            return x;
        };
        int ops = randInt(1, 300);
        for (int op = 0; op < ops; ++op) {
            int a = randInt(0, n - 1), b = randInt(0, n - 1);
            if (randInt(0, 1) == 0) {
                dsu.unite(a, b);
                int ra = refFind(a), rb = refFind(b);
                if (ra != rb) ref[ra] = rb;
            } else {
                assert(dsu.same(a, b) == (refFind(a) == refFind(b)));
            }
        }
        // Final consistency: connectivity must agree for every pair.
        // (The representative element may differ between implementations,
        //  so we compare components, not exact roots.)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                assert(dsu.same(i, j) == (refFind(i) == refFind(j)));
    }
    cout << "  OK (200 random runs)\n";
}

static void testRollbackDSU() {
    cout << "[RollbackDSU] stress test vs. plain DSU with undo\n";
    for (int iter = 0; iter < 200; ++iter) {
        int n = randInt(1, 40);
        RollbackDSU rb(n);
        // Reference: classic DSU with a stack of (a, b, oldParent) records.
        vector<int> parent(n);
        iota(parent.begin(), parent.end(), 0);
        vector<tuple<int, int, int>> hist;  // (child, oldParent)
        auto findRef = [&](int x) {
            while (parent[x] != x) x = parent[x];
            return x;
        };
        auto uniteRef = [&](int a, int b) {
            a = findRef(a);
            b = findRef(b);
            if (a == b) {
                hist.push_back({-1, -1, -1});
                return false;
            }
            hist.push_back({b, a, parent[b]});
            parent[b] = a;
            return true;
        };
        auto undoRef = [&]() {
            auto [child, root, oldP] = hist.back();
            hist.pop_back();
            if (child != -1) parent[child] = oldP;
        };
        int ops = randInt(1, 200);
        for (int op = 0; op < ops; ++op) {
            int a = randInt(0, n - 1), b = randInt(0, n - 1);
            int kind = randInt(0, 2);
            if (kind == 0) {
                assert(rb.unite(a, b) == uniteRef(a, b));
            } else if (kind == 1) {
                assert(rb.same(a, b) == (findRef(a) == findRef(b)));
            } else if (!hist.empty()) {
                rb.undo();
                undoRef();
            }
        }
        // Roll back everything; the DSU must be back to the identity.
        rb.rollback(0);
        for (int i = 0; i < n; ++i) assert(rb.find(i) == i);
    }
    cout << "  OK (200 random runs, full rollback verified)\n";
}

static void testMST() {
    cout << "[Kruskal vs Prim] stress test vs. brute-force MST\n";
    for (int iter = 0; iter < 300; ++iter) {
        int n = randInt(2, 8);
        int extra = randInt(0, min(8, n * (n - 1) / 2 - (n - 1)));
        vector<Edge> edges = randomConnectedGraph(n, extra);
        // Build adjacency for Prim.
        vector<vector<pair<int, ll>>> adj(n);
        for (auto& e : edges) {
            adj[e.u].push_back({e.v, e.w});
            adj[e.v].push_back({e.u, e.w});
        }
        auto [kEdges, kCost] = kruskalMST(n, edges);
        auto [pEdges, pCost] = primMST(n, adj);
        auto [bEdges, bCost] = bruteMST(n, edges);
        assert(kCost == bCost);
        assert(pCost == bCost);
        assert((int)kEdges.size() == n - 1);
        assert((int)pEdges.size() == n - 1);
        // The MST must be a spanning tree: n-1 edges, all vertices connected.
        DSU dsu(n);
        for (auto& e : kEdges) dsu.unite(e.u, e.v);
        assert(dsu.components() == 1);
    }
    cout << "  OK (300 random graphs, Kruskal == Prim == brute force)\n";
}

static void testTopoAndCycle() {
    cout << "[TopoSort & CycleDetection] stress test vs. brute force\n";
    for (int iter = 0; iter < 300; ++iter) {
        int n = randInt(1, 8);
        vector<vector<int>> adj(n);
        int m = randInt(0, min(16, n * (n - 1)));
        for (int i = 0; i < m; ++i) {
            int u = randInt(0, n - 1), v = randInt(0, n - 1);
            if (u == v) continue;
            adj[u].push_back(v);
        }
        bool cycle = bruteCycleDirected(n, adj);
        assert(hasCycleDirectedKahn(n, adj) == cycle);
        assert(hasCycleDirectedDFS(n, adj) == cycle);

        vector<int> k = topoKahn(n, adj);
        vector<int> d = topoDFS(n, adj);
        assert(k.empty() == cycle);
        assert(d.empty() == cycle);
        if (!cycle) {
            // Verify both orders are valid topological orders.
            auto valid = [&](const vector<int>& ord) {
                if ((int)ord.size() != n) return false;
                vector<int> pos(n);
                for (int i = 0; i < n; ++i) pos[ord[i]] = i;
                for (int u = 0; u < n; ++u)
                    for (int v : adj[u])
                        if (pos[u] > pos[v]) return false;
                return true;
            };
            assert(valid(k));
            assert(valid(d));
        }

        // Undirected cycle detection on the same vertex set.
        vector<pair<int, int>> uedges;
        set<pair<int, int>> seen;
        for (int u = 0; u < n; ++u)
            for (int v : adj[u])
                if (u < v && seen.insert({u, v}).second)
                    uedges.push_back({u, v});
        vector<vector<int>> uadj(n);
        for (auto [u, v] : uedges) {
            uadj[u].push_back(v);
            uadj[v].push_back(u);
        }
        bool ucycle = bruteCycleUndirected(n, uedges);
        assert(hasCycleUndirectedDSU(n, uedges) == ucycle);
        assert(hasCycleUndirectedDFS(n, uadj) == ucycle);
    }
    cout << "  OK (300 random graphs, Kahn == DFS == brute force)\n";
}

// ============================================================================
// Section 8: main()
// ============================================================================

int main() {
    cout << "============================================================\n";
    cout << " DSU / MST / Topological Sort / Cycle Detection\n";
    cout << " Randomized stress tests vs. brute-force references\n";
    cout << "============================================================\n\n";

    testDSU();
    testRollbackDSU();
    testMST();
    testTopoAndCycle();

    // A small worked example so the output is readable.
    cout << "\n------------------------------------------------------------\n";
    cout << " Worked example: 6-vertex weighted graph\n";
    cout << "------------------------------------------------------------\n";
    int n = 6;
    vector<Edge> edges = {
        {0, 1, 4}, {0, 2, 2}, {1, 2, 1}, {1, 3, 5},
        {2, 3, 8}, {2, 4, 10}, {3, 4, 2}, {3, 5, 6}, {4, 5, 3}};
    vector<vector<pair<int, ll>>> adj(n);
    for (auto& e : edges) {
        adj[e.u].push_back({e.v, e.w});
        adj[e.v].push_back({e.u, e.w});
    }
    auto [kEdges, kCost] = kruskalMST(n, edges);
    auto [pEdges, pCost] = primMST(n, adj);
    cout << " Kruskal MST cost = " << kCost << ", edges:";
    for (auto& e : kEdges) cout << " (" << e.u << "," << e.v << "," << e.w << ")";
    cout << "\n Prim    MST cost = " << pCost << ", edges:";
    for (auto& e : pEdges) cout << " (" << e.u << "," << e.v << "," << e.w << ")";
    cout << "\n (both must be 13 for this graph)\n";

    // Directed DAG example for topological sort.
    vector<vector<int>> dag(6);
    dag[0] = {1, 2};
    dag[1] = {3};
    dag[2] = {3, 4};
    dag[3] = {5};
    dag[4] = {5};
    auto k = topoKahn(6, dag);
    auto d = topoDFS(6, dag);
    cout << " Kahn topo order : ";
    for (int u : k) cout << u << " ";
    cout << "\n DFS  topo order : ";
    for (int u : d) cout << u << " ";
    cout << "\n (both are valid topological orders of the DAG)\n";

    // Cycle example.
    vector<vector<int>> cyc(3);
    cyc[0] = {1};
    cyc[1] = {2};
    cyc[2] = {0};
    cout << " Directed cycle 0->1->2->0 detected: "
         << (hasCycleDirectedKahn(3, cyc) ? "yes" : "no") << "\n";

    cout << "\nAll stress tests passed.\n";
    return 0;
}