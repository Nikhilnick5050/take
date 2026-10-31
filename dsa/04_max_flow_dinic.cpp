/*
 * ============================================================================
 *  04_max_flow_dinic.cpp
 * ============================================================================
 *  Dinic's maximum-flow algorithm and minimum s-t cut.
 *
 *  Algorithm outline:
 *    Repeat until the sink is unreachable in the residual graph:
 *      1. BFS from the source builds a "level graph" (distance in edges using
 *         only edges with residual capacity > 0).
 *      2. DFS sends a *blocking flow* along edges that go from level d to
 *         level d+1. A per-vertex "current arc" pointer (it_[v]) makes sure
 *         every edge is dead-ended at most once per phase.
 *    Complexity: O(V^2 E) in general, O(E sqrt(V)) on unit-capacity
 *    bipartite graphs, and much faster in practice.
 *
 *  Min cut (max-flow min-cut theorem):
 *    After the final BFS, S = vertices still reachable from the source in the
 *    residual graph. Every original edge from S to V\S is saturated and the
 *    sum of their capacities equals the max flow.
 *
 *  Included extras:
 *    - flow_on_edge(id)     : flow routed over a specific added edge
 *    - min_cut()            : source side vertices and the cut edges
 *    - bipartite matching   : built on top of the flow network
 *    - EdmondsKarp          : simple O(V E^2) algorithm used for cross-checks
 *    - flow conservation checker
 *
 *  Build:  g++ -std=c++17 -Wall -Wextra -O2 04_max_flow_dinic.cpp -o dinic
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using i64 = long long;

// ---------------------------------------------------------------------------
//  Dinic
// ---------------------------------------------------------------------------
class Dinic {
public:
    static constexpr i64 INF = std::numeric_limits<i64>::max() / 4;

    struct Edge {
        int to;       // head vertex
        i64 cap;      // residual capacity
        i64 orig;     // original capacity (0 for reverse edges)
    };

    explicit Dinic(int n) : n_(n), adj_(n), level_(n), it_(n) {}

    int vertices() const { return n_; }

    // Adds a directed edge u -> v with the given capacity and returns its id.
    // The reverse (residual) edge is stored at id ^ 1.
    int add_edge(int u, int v, i64 cap) {
        assert(0 <= u && u < n_ && 0 <= v && v < n_ && cap >= 0);
        int id = static_cast<int>(edges_.size());
        edges_.push_back({v, cap, cap});
        edges_.push_back({u, 0, 0});
        adj_[u].push_back(id);
        adj_[v].push_back(id + 1);
        return id;
    }

    // Computes the max flow from s to t. May be called again after adding
    // more edges: it continues from the current residual graph.
    i64 max_flow(int s, int t) {
        if (s == t) return 0;
        i64 flow = 0;
        while (bfs(s, t)) {
            std::fill(it_.begin(), it_.end(), 0);
            while (i64 pushed = dfs(s, t, INF)) flow += pushed;
            ++phases_;
        }
        last_source_ = s;
        return flow;
    }

    int phases() const { return phases_; }

    // Flow currently routed over the edge returned by add_edge.
    i64 flow_on_edge(int id) const { return edges_[id].orig - edges_[id].cap; }
    i64 capacity_of_edge(int id) const { return edges_[id].orig; }
    int edge_from(int id) const { return edges_[id ^ 1].to; }
    int edge_to(int id) const { return edges_[id].to; }

    // After max_flow(s, t): the set of vertices on the source side of a
    // minimum cut (those reachable from s in the residual graph).
    std::vector<bool> min_cut_source_side() const {
        std::vector<bool> seen(n_, false);
        std::queue<int> q;
        seen[last_source_] = true;
        q.push(last_source_);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int id : adj_[u]) {
                if (edges_[id].cap > 0 && !seen[edges_[id].to]) {
                    seen[edges_[id].to] = true;
                    q.push(edges_[id].to);
                }
            }
        }
        return seen;
    }

    // Ids of the original edges crossing the minimum cut (S -> T).
    std::vector<int> min_cut_edges() const {
        std::vector<bool> inS = min_cut_source_side();
        std::vector<int> cut;
        for (int id = 0; id < static_cast<int>(edges_.size()); id += 2) {
            int u = edges_[id ^ 1].to, v = edges_[id].to;
            if (inS[u] && !inS[v] && edges_[id].orig > 0) cut.push_back(id);
        }
        return cut;
    }

    // Sum of capacities of the min cut edges (should equal the max flow).
    i64 min_cut_value() const {
        i64 total = 0;
        for (int id : min_cut_edges()) total += edges_[id].orig;
        return total;
    }

    // Verify capacity constraints and flow conservation for max_flow(s, t).
    bool check_flow(int s, int t, i64 expected) const {
        std::vector<i64> balance(n_, 0);
        for (int id = 0; id < static_cast<int>(edges_.size()); id += 2) {
            i64 f = flow_on_edge(id);
            if (f < 0 || f > edges_[id].orig) return false;  // capacity
            balance[edge_from(id)] -= f;
            balance[edge_to(id)] += f;
        }
        for (int v = 0; v < n_; ++v) {
            if (v == s) { if (balance[v] != -expected) return false; }
            else if (v == t) { if (balance[v] != expected) return false; }
            else if (balance[v] != 0) return false;  // conservation
        }
        return true;
    }

private:
    int n_;
    std::vector<Edge> edges_;
    std::vector<std::vector<int>> adj_;  // edge ids per vertex
    std::vector<int> level_;
    std::vector<int> it_;                // current-arc pointers
    int phases_ = 0;
    int last_source_ = 0;

    // Build the level graph. Returns true if t is reachable.
    bool bfs(int s, int t) {
        std::fill(level_.begin(), level_.end(), -1);
        std::queue<int> q;
        level_[s] = 0;
        q.push(s);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int id : adj_[u]) {
                const Edge& e = edges_[id];
                if (e.cap > 0 && level_[e.to] < 0) {
                    level_[e.to] = level_[u] + 1;
                    q.push(e.to);
                }
            }
        }
        return level_[t] >= 0;
    }

    // Send up to `limit` units from u to t along the level graph.
    i64 dfs(int u, int t, i64 limit) {
        if (u == t) return limit;
        for (int& i = it_[u]; i < static_cast<int>(adj_[u].size()); ++i) {
            int id = adj_[u][i];
            Edge& e = edges_[id];
            if (e.cap <= 0 || level_[e.to] != level_[u] + 1) continue;
            i64 pushed = dfs(e.to, t, std::min(limit, e.cap));
            if (pushed > 0) {
                e.cap -= pushed;
                edges_[id ^ 1].cap += pushed;
                return pushed;
            }
            // pushed == 0: this arc is a dead end; ++i skips it for the phase
        }
        return 0;
    }
};

// ---------------------------------------------------------------------------
//  Edmonds-Karp (reference implementation, adjacency matrix)
// ---------------------------------------------------------------------------
static i64 edmondsKarp(std::vector<std::vector<i64>> cap, int s, int t) {
    int n = static_cast<int>(cap.size());
    i64 flow = 0;
    while (true) {
        std::vector<int> parent(n, -1);
        parent[s] = s;
        std::queue<int> q;
        q.push(s);
        while (!q.empty() && parent[t] < 0) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; ++v)
                if (parent[v] < 0 && cap[u][v] > 0) {
                    parent[v] = u;
                    q.push(v);
                }
        }
        if (parent[t] < 0) break;
        i64 bottleneck = Dinic::INF;
        for (int v = t; v != s; v = parent[v])
            bottleneck = std::min(bottleneck, cap[parent[v]][v]);
        for (int v = t; v != s; v = parent[v]) {
            cap[parent[v]][v] -= bottleneck;
            cap[v][parent[v]] += bottleneck;
        }
        flow += bottleneck;
    }
    return flow;
}

// ---------------------------------------------------------------------------
//  Bipartite matching via flow:  source -> left (cap 1) -> right -> sink
// ---------------------------------------------------------------------------
struct BipartiteMatching {
    int left, right;
    std::vector<std::pair<int, int>> pairs;  // allowed (l, r) pairs

    // Returns matching size and fills `match` with (l, r) pairs.
    int solve(std::vector<std::pair<int, int>>& match) const {
        int S = left + right, T = S + 1;
        Dinic d(left + right + 2);
        for (int l = 0; l < left; ++l) d.add_edge(S, l, 1);
        for (int r = 0; r < right; ++r) d.add_edge(left + r, T, 1);
        std::vector<int> ids;
        for (auto [l, r] : pairs) ids.push_back(d.add_edge(l, left + r, 1));
        int size = static_cast<int>(d.max_flow(S, T));
        match.clear();
        for (size_t i = 0; i < pairs.size(); ++i)
            if (d.flow_on_edge(ids[i]) == 1) match.push_back(pairs[i]);
        return size;
    }
};

// ---------------------------------------------------------------------------
//  Test utilities
// ---------------------------------------------------------------------------
static int g_failures = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            ++g_failures;                                                 \
            std::cout << "  [FAIL] line " << __LINE__ << ": " #cond "\n"; \
        }                                                                 \
    } while (0)

// ---------------------------------------------------------------------------
//  Tests
// ---------------------------------------------------------------------------
static void testClassic() {
    std::cout << "== Classic CLRS network (expected 23) ==\n";
    // Vertices: 0=s 1=v1 2=v2 3=v3 4=v4 5=t
    Dinic d(6);
    d.add_edge(0, 1, 16);
    d.add_edge(0, 2, 13);
    d.add_edge(1, 2, 10);
    d.add_edge(2, 1, 4);
    d.add_edge(1, 3, 12);
    d.add_edge(3, 2, 9);
    d.add_edge(2, 4, 14);
    d.add_edge(4, 3, 7);
    d.add_edge(3, 5, 20);
    d.add_edge(4, 5, 4);
    i64 f = d.max_flow(0, 5);
    std::cout << "  max flow        : " << f << " (phases=" << d.phases() << ")\n";
    CHECK(f == 23);
    CHECK(d.check_flow(0, 5, f));
    CHECK(d.min_cut_value() == f);

    auto side = d.min_cut_source_side();
    std::cout << "  source side     : ";
    for (int v = 0; v < 6; ++v)
        if (side[v]) std::cout << v << " ";
    std::cout << "\n  cut edges       : ";
    for (int id : d.min_cut_edges())
        std::cout << d.edge_from(id) << "->" << d.edge_to(id) << "("
                  << d.capacity_of_edge(id) << ") ";
    std::cout << "\n";
    CHECK(side[0]);
    CHECK(!side[5]);
}

static void testSmallCases() {
    std::cout << "== Small / degenerate cases ==\n";
    {   // single edge
        Dinic d(2);
        d.add_edge(0, 1, 5);
        CHECK(d.max_flow(0, 1) == 5);
    }
    {   // s == t
        Dinic d(3);
        d.add_edge(0, 1, 5);
        CHECK(d.max_flow(1, 1) == 0);
    }
    {   // disconnected
        Dinic d(4);
        d.add_edge(0, 1, 5);
        d.add_edge(2, 3, 5);
        CHECK(d.max_flow(0, 3) == 0);
        CHECK(d.min_cut_value() == 0);
        auto side = d.min_cut_source_side();
        CHECK(side[0] && side[1] && !side[2] && !side[3]);
    }
    {   // parallel edges add up
        Dinic d(2);
        d.add_edge(0, 1, 3);
        d.add_edge(0, 1, 4);
        d.add_edge(0, 1, 5);
        CHECK(d.max_flow(0, 1) == 12);
    }
    {   // bottleneck in the middle
        Dinic d(4);
        d.add_edge(0, 1, 100);
        d.add_edge(1, 2, 1);
        d.add_edge(2, 3, 100);
        CHECK(d.max_flow(0, 3) == 1);
        auto cut = d.min_cut_edges();
        CHECK(cut.size() == 1);
        CHECK(d.edge_from(cut[0]) == 1 && d.edge_to(cut[0]) == 2);
    }
    {   // flow must use a back edge (cancelling flow): classic diamond
        Dinic d(4);
        d.add_edge(0, 1, 1);
        d.add_edge(0, 2, 1);
        d.add_edge(1, 2, 1);
        d.add_edge(1, 3, 1);
        d.add_edge(2, 3, 1);
        CHECK(d.max_flow(0, 3) == 2);
    }
    {   // incremental: add edges after a first run
        Dinic d(3);
        d.add_edge(0, 1, 4);
        d.add_edge(1, 2, 2);
        CHECK(d.max_flow(0, 2) == 2);
        d.add_edge(1, 2, 5);
        CHECK(d.max_flow(0, 2) == 2);  // 2 more units pass (4 total in edge 0->1)
    }
    {   // zero capacity edges are harmless
        Dinic d(3);
        d.add_edge(0, 1, 0);
        d.add_edge(1, 2, 10);
        CHECK(d.max_flow(0, 2) == 0);
    }
    std::cout << "  small cases done\n";
}

static void testBipartite() {
    std::cout << "== Bipartite matching ==\n";
    BipartiteMatching bm;
    bm.left = 4;
    bm.right = 4;
    bm.pairs = {{0, 0}, {0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 2}, {3, 2}, {3, 3}};
    std::vector<std::pair<int, int>> match;
    int size = bm.solve(match);
    std::cout << "  matching size   : " << size << " ->";
    for (auto [l, r] : match) std::cout << " (" << l << "," << r << ")";
    std::cout << "\n";
    CHECK(size == 4);
    // Validate: each vertex used at most once.
    std::vector<int> usedL(4, 0), usedR(4, 0);
    for (auto [l, r] : match) { ++usedL[l]; ++usedR[r]; }
    for (int i = 0; i < 4; ++i) CHECK(usedL[i] <= 1 && usedR[i] <= 1);

    // Hall violation: three left vertices share two right vertices.
    BipartiteMatching bad;
    bad.left = 3;
    bad.right = 3;
    bad.pairs = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}};
    CHECK(bad.solve(match) == 2);
}

static void testGridCut() {
    std::cout << "== Min cut on a layered graph ==\n";
    // Two parallel chains s -> a -> b -> t, with bottlenecks 3 and 4.
    Dinic d(6);
    int s = 0, a1 = 1, b1 = 2, a2 = 3, b2 = 4, t = 5;
    d.add_edge(s, a1, 10);
    d.add_edge(a1, b1, 3);
    d.add_edge(b1, t, 10);
    d.add_edge(s, a2, 10);
    d.add_edge(a2, b2, 10);
    d.add_edge(b2, t, 4);
    i64 f = d.max_flow(s, t);
    std::cout << "  flow=" << f << " cut value=" << d.min_cut_value() << "\n";
    CHECK(f == 7);
    CHECK(d.min_cut_value() == 7);
    CHECK(d.min_cut_edges().size() == 2);
    CHECK(d.check_flow(s, t, f));
}

static void testRandomVsEdmondsKarp() {
    std::cout << "== Randomized Dinic vs Edmonds-Karp ==\n";
    std::mt19937 rng(77);
    int bad = 0, cases = 0;
    for (int iter = 0; iter < 300; ++iter) {
        int n = 2 + static_cast<int>(rng() % 12);
        int m = static_cast<int>(rng() % (n * 3));
        Dinic d(n);
        std::vector<std::vector<i64>> mat(n, std::vector<i64>(n, 0));
        for (int i = 0; i < m; ++i) {
            int u = static_cast<int>(rng() % n), v = static_cast<int>(rng() % n);
            if (u == v) continue;
            i64 c = static_cast<i64>(rng() % 20);
            d.add_edge(u, v, c);
            mat[u][v] += c;  // parallel edges merge in the matrix version
        }
        int s = 0, t = n - 1;
        i64 f1 = d.max_flow(s, t);
        i64 f2 = edmondsKarp(mat, s, t);
        ++cases;
        if (f1 != f2) ++bad;
        if (!d.check_flow(s, t, f1)) ++bad;
        if (d.min_cut_value() != f1) ++bad;
    }
    CHECK(bad == 0);
    std::cout << "  cases=" << cases << " problems=" << bad << "\n";
}

static void testLarge() {
    std::cout << "== Large unit-capacity bipartite graph ==\n";
    const int N = 2000;
    std::mt19937 rng(31);
    BipartiteMatching bm;
    bm.left = N;
    bm.right = N;
    // Guarantee a perfect matching (i,i) and add random extra edges.
    for (int i = 0; i < N; ++i) bm.pairs.push_back({i, i});
    for (int i = 0; i < 6 * N; ++i)
        bm.pairs.push_back({static_cast<int>(rng() % N), static_cast<int>(rng() % N)});
    std::vector<std::pair<int, int>> match;
    int size = bm.solve(match);
    std::cout << "  matching size   : " << size << " of " << N << "\n";
    CHECK(size == N);

    std::cout << "== Large layered network ==\n";
    const int L = 50, W = 40;  // L layers of W vertices
    Dinic d(L * W + 2);
    int s = L * W, t = s + 1;
    for (int j = 0; j < W; ++j) {
        d.add_edge(s, j, 1000);
        d.add_edge((L - 1) * W + j, t, 1000);
    }
    for (int l = 0; l + 1 < L; ++l)
        for (int j = 0; j < W; ++j)
            for (int k = 0; k < 3; ++k)
                d.add_edge(l * W + j, (l + 1) * W + static_cast<int>(rng() % W),
                           1 + static_cast<i64>(rng() % 50));
    i64 f = d.max_flow(s, t);
    std::cout << "  flow=" << f << " phases=" << d.phases() << "\n";
    CHECK(d.check_flow(s, t, f));
    CHECK(d.min_cut_value() == f);
}

int main() {
    std::cout << "Dinic max flow and min cut\n\n";
    testClassic();
    testSmallCases();
    testBipartite();
    testGridCut();
    testRandomVsEdmondsKarp();
    testLarge();
    std::cout << "\n" << (g_failures == 0 ? "ALL TESTS PASSED"
                                          : "SOME TESTS FAILED")
              << " (failures: " << g_failures << ")\n";
    return g_failures == 0 ? 0 : 1;
}
