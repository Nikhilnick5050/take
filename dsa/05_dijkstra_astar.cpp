/*
 * ============================================================================
 *  File    : 05_dijkstra_astar.cpp
 *  Topic   : Shortest paths - Dijkstra (binary heap) and A* on a grid
 *  Standard: C++17
 *  Build   : g++ -std=c++17 -Wall -Wextra 05_dijkstra_astar.cpp -o dijkstra
 * ----------------------------------------------------------------------------
 *  Contents
 *    1. Weighted directed graph (adjacency list)
 *    2. Dijkstra with std::priority_queue (lazy deletion)  O((V+E) log V)
 *    3. Dijkstra with an indexed min-heap (decrease-key)   O((V+E) log V)
 *    4. Path reconstruction from a parent array
 *    5. Dijkstra variants: multi-source, count of shortest paths,
 *       early exit to a single target
 *    6. Grid world with obstacles and 4/8-direction movement
 *    7. A* search with Manhattan / Chebyshev / Octile / Euclid heuristics
 *    8. Cross-checks: A* cost must equal Dijkstra cost on the same grid
 *
 *  Dijkstra requires non-negative edge weights. A* requires an admissible
 *  (never over-estimating) heuristic; with a consistent heuristic each node
 *  is expanded at most once, which is what the closed set below relies on.
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

using ll = long long;
static const ll INF = numeric_limits<ll>::max() / 4;

// ============================================================================
// Section 1: Graph
// ============================================================================

struct Edge {
    int to;
    ll w;
};

class Graph {
public:
    explicit Graph(int n) : n_(n), adj_(n) {}

    // Add a directed edge u -> v with weight w (must be >= 0 for Dijkstra).
    void addEdge(int u, int v, ll w) { adj_[u].push_back({v, w}); }

    // Add an undirected edge as two directed edges.
    void addUndirected(int u, int v, ll w) {
        addEdge(u, v, w);
        addEdge(v, u, w);
    }

    int size() const { return n_; }
    const vector<Edge>& neighbors(int u) const { return adj_[u]; }

private:
    int n_;
    vector<vector<Edge>> adj_;
};

// Result of a single-source shortest path run.
struct ShortestPaths {
    vector<ll> dist;    // dist[v] = INF when v is unreachable
    vector<int> parent; // parent[v] = predecessor on a shortest path, -1 if none
};

// ============================================================================
// Section 2: Dijkstra with std::priority_queue (lazy deletion)
// ============================================================================
// We push a new (dist,node) pair every time we find a better distance. Stale
// entries are skipped when popped (d > dist[u]). Simple and fast in practice.

ShortestPaths dijkstra(const Graph& g, int src) {
    int n = g.size();
    ShortestPaths r{vector<ll>(n, INF), vector<int>(n, -1)};
    using P = pair<ll, int>;
    priority_queue<P, vector<P>, greater<P>> pq; // min-heap on distance

    r.dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > r.dist[u]) continue; // stale entry
        for (const Edge& e : g.neighbors(u)) {
            ll nd = d + e.w;
            if (nd < r.dist[e.to]) {
                r.dist[e.to] = nd;
                r.parent[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }
    return r;
}

// ============================================================================
// Section 3: Indexed min-heap supporting decrease-key
// ============================================================================
// Keeps at most one entry per node. pos_[v] gives the heap index of node v
// (or -1 if absent), which makes decreaseKey O(log n).

class IndexedMinHeap {
public:
    explicit IndexedMinHeap(int n) : key_(n, INF), pos_(n, -1) {}

    bool empty() const { return heap_.empty(); }
    bool contains(int v) const { return pos_[v] != -1; }

    // Insert node v with key k, or lower its key if already present.
    void pushOrDecrease(int v, ll k) {
        if (pos_[v] == -1) {
            key_[v] = k;
            pos_[v] = (int)heap_.size();
            heap_.push_back(v);
            siftUp(pos_[v]);
        } else if (k < key_[v]) {
            key_[v] = k;
            siftUp(pos_[v]);
        }
    }

    // Remove and return the node with the smallest key.
    pair<int, ll> popMin() {
        int v = heap_[0];
        ll k = key_[v];
        swapNodes(0, (int)heap_.size() - 1);
        pos_[v] = -1;
        heap_.pop_back();
        if (!heap_.empty()) siftDown(0);
        return {v, k};
    }

private:
    vector<int> heap_; // heap of node ids
    vector<ll> key_;   // key per node id
    vector<int> pos_;  // position per node id

    void swapNodes(int i, int j) {
        swap(heap_[i], heap_[j]);
        pos_[heap_[i]] = i;
        pos_[heap_[j]] = j;
    }
    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (key_[heap_[i]] < key_[heap_[p]]) {
                swapNodes(i, p);
                i = p;
            } else break;
        }
    }
    void siftDown(int i) {
        int n = (int)heap_.size();
        while (true) {
            int l = 2 * i + 1, r = l + 1, m = i;
            if (l < n && key_[heap_[l]] < key_[heap_[m]]) m = l;
            if (r < n && key_[heap_[r]] < key_[heap_[m]]) m = r;
            if (m == i) break;
            swapNodes(i, m);
            i = m;
        }
    }
};

ShortestPaths dijkstraIndexed(const Graph& g, int src) {
    int n = g.size();
    ShortestPaths r{vector<ll>(n, INF), vector<int>(n, -1)};
    IndexedMinHeap h(n);
    r.dist[src] = 0;
    h.pushOrDecrease(src, 0);
    while (!h.empty()) {
        auto [u, d] = h.popMin();
        for (const Edge& e : g.neighbors(u)) {
            ll nd = d + e.w;
            if (nd < r.dist[e.to]) {
                r.dist[e.to] = nd;
                r.parent[e.to] = u;
                h.pushOrDecrease(e.to, nd);
            }
        }
    }
    return r;
}

// ============================================================================
// Section 4: Path reconstruction
// ============================================================================

vector<int> buildPath(const vector<int>& parent, int src, int target) {
    vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
        if (v == src) break;
    }
    reverse(path.begin(), path.end());
    if (path.empty() || path.front() != src) return {}; // unreachable
    return path;
}

template <class T>
void printVec(const string& label, const vector<T>& v) {
    cout << label;
    for (size_t i = 0; i < v.size(); ++i) cout << (i ? " " : "") << v[i];
    cout << "\n";
}

// ============================================================================
// Section 5: Dijkstra variants
// ============================================================================

// Multi-source: all sources start at distance 0 (a "super source").
vector<ll> dijkstraMultiSource(const Graph& g, const vector<int>& sources) {
    vector<ll> dist(g.size(), INF);
    using P = pair<ll, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    for (int s : sources) {
        dist[s] = 0;
        pq.push({0, s});
    }
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : g.neighbors(u))
            if (d + e.w < dist[e.to]) {
                dist[e.to] = d + e.w;
                pq.push({dist[e.to], e.to});
            }
    }
    return dist;
}

// Counts the number of distinct shortest paths from src to each node
// (modulo 1e9+7). Works because zero-weight edges are not used in the demo.
vector<ll> countShortestPaths(const Graph& g, int src) {
    const ll MOD = 1'000'000'007LL;
    int n = g.size();
    vector<ll> dist(n, INF), ways(n, 0);
    using P = pair<ll, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src] = 0;
    ways[src] = 1;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : g.neighbors(u)) {
            ll nd = d + e.w;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                ways[e.to] = ways[u];
                pq.push({nd, e.to});
            } else if (nd == dist[e.to]) {
                ways[e.to] = (ways[e.to] + ways[u]) % MOD;
            }
        }
    }
    return ways;
}

// Early exit: stop as soon as the target is settled.
ll dijkstraTarget(const Graph& g, int src, int target, int& settledCount) {
    vector<ll> dist(g.size(), INF);
    vector<char> done(g.size(), 0);
    using P = pair<ll, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src] = 0;
    pq.push({0, src});
    settledCount = 0;
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (done[u]) continue;
        done[u] = 1;
        ++settledCount;
        if (u == target) return d;
        for (const Edge& e : g.neighbors(u))
            if (d + e.w < dist[e.to]) {
                dist[e.to] = d + e.w;
                pq.push({dist[e.to], e.to});
            }
    }
    return INF;
}

// ============================================================================
// Section 6: Grid world
// ============================================================================
// '.' is free, '#' is a wall. Straight moves cost 10 and diagonal moves cost
// 14 (integer approximation of 10*sqrt(2)); using integers avoids float
// comparison trouble when checking A* against Dijkstra.

struct Grid {
    int rows = 0, cols = 0;
    vector<string> cells;

    explicit Grid(vector<string> c) : rows((int)c.size()), cols((int)c[0].size()), cells(move(c)) {}

    bool free(int r, int c) const {
        return r >= 0 && c >= 0 && r < rows && c < cols && cells[r][c] != '#';
    }
    int id(int r, int c) const { return r * cols + c; }
};

struct Move {
    int dr, dc;
    ll cost;
};

static const vector<Move> MOVES4 = {{1, 0, 10}, {-1, 0, 10}, {0, 1, 10}, {0, -1, 10}};
static const vector<Move> MOVES8 = {{1, 0, 10},  {-1, 0, 10}, {0, 1, 10},  {0, -1, 10},
                                    {1, 1, 14},  {1, -1, 14}, {-1, 1, 14}, {-1, -1, 14}};

// Build an explicit Graph from the grid so that Dijkstra can be run on it.
// Diagonal moves are not allowed to cut corners of walls.
Graph gridToGraph(const Grid& g, const vector<Move>& moves) {
    Graph gr(g.rows * g.cols);
    for (int r = 0; r < g.rows; ++r)
        for (int c = 0; c < g.cols; ++c) {
            if (!g.free(r, c)) continue;
            for (const Move& m : moves) {
                int nr = r + m.dr, nc = c + m.dc;
                if (!g.free(nr, nc)) continue;
                if (m.dr != 0 && m.dc != 0 && (!g.free(r + m.dr, c) || !g.free(r, c + m.dc)))
                    continue; // no corner cutting
                gr.addEdge(g.id(r, c), g.id(nr, nc), m.cost);
            }
        }
    return gr;
}

// ============================================================================
// Section 7: A* search
// ============================================================================

enum class Heuristic { Zero, Manhattan, Chebyshev, Octile, Euclid };

const char* heuristicName(Heuristic h) {
    switch (h) {
        case Heuristic::Zero: return "Zero(=Dijkstra)";
        case Heuristic::Manhattan: return "Manhattan";
        case Heuristic::Chebyshev: return "Chebyshev";
        case Heuristic::Octile: return "Octile";
        case Heuristic::Euclid: return "Euclid";
    }
    return "?";
}

// Heuristic estimate in the same units as move costs (10 / 14).
ll heuristicValue(Heuristic h, int r, int c, int tr, int tc) {
    ll dr = abs(r - tr), dc = abs(c - tc);
    switch (h) {
        case Heuristic::Zero: return 0;
        case Heuristic::Manhattan: return 10 * (dr + dc);
        case Heuristic::Chebyshev: return 10 * max(dr, dc);
        case Heuristic::Octile: return 10 * (dr + dc) - 6 * min(dr, dc); // 14*min + 10*(max-min)
        case Heuristic::Euclid: return (ll)floor(10.0 * sqrt((double)(dr * dr + dc * dc)));
    }
    return 0;
}

struct AStarResult {
    ll cost = INF;                       // INF when no path exists
    vector<pair<int, int>> path;         // cells from start to goal
    int expanded = 0;                    // number of nodes popped & expanded
};

AStarResult astar(const Grid& g, pair<int, int> start, pair<int, int> goal,
                  const vector<Move>& moves, Heuristic h) {
    AStarResult res;
    if (!g.free(start.first, start.second) || !g.free(goal.first, goal.second)) return res;

    int n = g.rows * g.cols;
    vector<ll> gScore(n, INF);
    vector<int> parent(n, -1);
    vector<char> closed(n, 0);

    // Entry: (f, h, id). Ties on f are broken in favour of smaller h, which
    // makes the search dive towards the goal instead of fanning out.
    using E = tuple<ll, ll, int>;
    priority_queue<E, vector<E>, greater<E>> open;

    int s = g.id(start.first, start.second), t = g.id(goal.first, goal.second);
    gScore[s] = 0;
    ll h0 = heuristicValue(h, start.first, start.second, goal.first, goal.second);
    open.push({h0, h0, s});

    while (!open.empty()) {
        auto [f, hh, u] = open.top();
        (void)f; (void)hh;
        open.pop();
        if (closed[u]) continue;
        closed[u] = 1;
        ++res.expanded;
        if (u == t) break;
        int r = u / g.cols, c = u % g.cols;
        for (const Move& m : moves) {
            int nr = r + m.dr, nc = c + m.dc;
            if (!g.free(nr, nc)) continue;
            if (m.dr != 0 && m.dc != 0 && (!g.free(r + m.dr, c) || !g.free(r, c + m.dc))) continue;
            int v = g.id(nr, nc);
            if (closed[v]) continue;
            ll ng = gScore[u] + m.cost;
            if (ng < gScore[v]) {
                gScore[v] = ng;
                parent[v] = u;
                ll hv = heuristicValue(h, nr, nc, goal.first, goal.second);
                open.push({ng + hv, hv, v});
            }
        }
    }

    if (gScore[t] >= INF) return res;
    res.cost = gScore[t];
    for (int v = t; v != -1; v = parent[v]) res.path.push_back({v / g.cols, v % g.cols});
    reverse(res.path.begin(), res.path.end());
    return res;
}

// Draw the grid with the path marked by '*', S and G for endpoints.
void printGridWithPath(const Grid& g, const AStarResult& r, pair<int, int> s, pair<int, int> t) {
    vector<string> out = g.cells;
    for (auto [pr, pc] : r.path) out[pr][pc] = '*';
    out[s.first][s.second] = 'S';
    out[t.first][t.second] = 'G';
    for (const string& line : out) cout << "    " << line << "\n";
}

// ============================================================================
// Section 8: Tests
// ============================================================================

void testSmallGraph() {
    cout << "=== Test 1: small graph, both Dijkstra implementations ===\n";
    Graph g(6);
    g.addEdge(0, 1, 7);
    g.addEdge(0, 2, 9);
    g.addEdge(0, 5, 14);
    g.addEdge(1, 2, 10);
    g.addEdge(1, 3, 15);
    g.addEdge(2, 3, 11);
    g.addEdge(2, 5, 2);
    g.addEdge(3, 4, 6);
    g.addEdge(5, 4, 9);

    auto a = dijkstra(g, 0);
    auto b = dijkstraIndexed(g, 0);
    printVec("  lazy    dist: ", a.dist);
    printVec("  indexed dist: ", b.dist);
    assert(a.dist == b.dist);
    vector<ll> expected = {0, 7, 9, 20, 20, 11};
    assert(a.dist == expected);
    auto p = buildPath(a.parent, 0, 4);
    printVec("  path 0->4: ", p);
    assert((p == vector<int>{0, 2, 5, 4}));
    cout << "  OK\n\n";
}

void testUnreachable() {
    cout << "=== Test 2: unreachable nodes ===\n";
    Graph g(4);
    g.addEdge(0, 1, 3);
    g.addEdge(2, 3, 1);
    auto r = dijkstra(g, 0);
    cout << "  dist to 2: " << (r.dist[2] == INF ? "INF" : to_string(r.dist[2])) << "\n";
    assert(r.dist[2] == INF);
    assert(buildPath(r.parent, 0, 3).empty());
    cout << "  OK\n\n";
}

void testRandomCrossCheck() {
    cout << "=== Test 3: random graphs, lazy vs indexed heap vs Bellman-Ford ===\n";
    uint64_t seed = 12345;
    auto rnd = [&seed]() {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return (int)(seed >> 33);
    };
    int trials = 50;
    for (int t = 0; t < trials; ++t) {
        int n = 2 + rnd() % 30;
        int m = rnd() % (n * 4);
        Graph g(n);
        vector<tuple<int, int, ll>> edges;
        for (int i = 0; i < m; ++i) {
            int u = rnd() % n, v = rnd() % n;
            ll w = rnd() % 50;
            g.addEdge(u, v, w);
            edges.push_back({u, v, w});
        }
        // Bellman-Ford as an independent reference.
        vector<ll> bf(n, INF);
        bf[0] = 0;
        for (int it = 0; it < n; ++it)
            for (auto [u, v, w] : edges)
                if (bf[u] < INF && bf[u] + w < bf[v]) bf[v] = bf[u] + w;
        auto a = dijkstra(g, 0);
        auto b = dijkstraIndexed(g, 0);
        assert(a.dist == bf);
        assert(b.dist == bf);
    }
    cout << "  " << trials << " random graphs agree\n  OK\n\n";
}

void testVariants() {
    cout << "=== Test 4: multi-source, path counting, early exit ===\n";
    // A diamond lattice with two equally short routes 0->3 and 0->4->... etc.
    Graph g(5);
    g.addUndirected(0, 1, 1);
    g.addUndirected(0, 2, 1);
    g.addUndirected(1, 3, 1);
    g.addUndirected(2, 3, 1);
    g.addUndirected(3, 4, 5);
    auto ways = countShortestPaths(g, 0);
    printVec("  #shortest paths from 0: ", ways);
    assert(ways[3] == 2 && ways[4] == 2);

    auto ms = dijkstraMultiSource(g, {0, 4});
    printVec("  multi-source {0,4} dist: ", ms);
    assert(ms[3] == 2 && ms[1] == 1);

    int settled = 0;
    ll d = dijkstraTarget(g, 0, 1, settled);
    cout << "  dist(0,1)=" << d << " settled nodes=" << settled << " of 5\n";
    assert(d == 1 && settled < 5);
    cout << "  OK\n\n";
}

void testAStarBasic() {
    cout << "=== Test 5: A* on a maze (8-direction, octile heuristic) ===\n";
    Grid g({
        "..........",
        ".########.",
        ".#......#.",
        ".#.####.#.",
        ".#.#..#.#.",
        ".#.#.##.#.",
        ".#.#....#.",
        ".#.######.",
        ".#........",
        "..........",
    });
    pair<int, int> s{0, 0}, t{4, 4};
    auto r = astar(g, s, t, MOVES8, Heuristic::Octile);
    cout << "  cost=" << r.cost << " steps=" << r.path.size() << " expanded=" << r.expanded << "\n";
    printGridWithPath(g, r, s, t);
    assert(r.cost < INF);
    assert(r.path.front() == s && r.path.back() == t);
    cout << "  OK\n\n";
}

void testAStarNoPath() {
    cout << "=== Test 6: A* when goal is walled in ===\n";
    Grid g({
        ".....",
        ".###.",
        ".#G#.",
        ".###.",
        ".....",
    });
    auto r = astar(g, {0, 0}, {2, 2}, MOVES4, Heuristic::Manhattan);
    cout << "  cost=" << (r.cost == INF ? string("INF") : to_string(r.cost)) << "\n";
    assert(r.cost == INF && r.path.empty());
    cout << "  OK\n\n";
}

void testAStarVsDijkstra() {
    cout << "=== Test 7: A* vs Dijkstra on random grids, all heuristics ===\n";
    uint64_t seed = 987654321;
    auto rnd = [&seed]() {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return (int)(seed >> 33);
    };
    const Heuristic hs4[] = {Heuristic::Zero, Heuristic::Manhattan};
    const Heuristic hs8[] = {Heuristic::Zero, Heuristic::Octile, Heuristic::Chebyshev,
                             Heuristic::Euclid};
    long long totalExpandedZero = 0, totalExpandedBest = 0;
    int solvable = 0;
    for (int t = 0; t < 40; ++t) {
        int R = 12, C = 15;
        vector<string> cells(R, string(C, '.'));
        for (int r = 0; r < R; ++r)
            for (int c = 0; c < C; ++c)
                if (rnd() % 100 < 28) cells[r][c] = '#';
        cells[0][0] = '.';
        cells[R - 1][C - 1] = '.';
        Grid g(cells);

        for (int eight = 0; eight < 2; ++eight) {
            const auto& moves = eight ? MOVES8 : MOVES4;
            Graph gr = gridToGraph(g, moves);
            auto dj = dijkstra(gr, g.id(0, 0));
            ll ref = dj.dist[g.id(R - 1, C - 1)];
            if (eight) {
                for (Heuristic h : hs8) {
                    auto r = astar(g, {0, 0}, {R - 1, C - 1}, moves, h);
                    assert(r.cost == ref);
                    if (h == Heuristic::Zero) totalExpandedZero += r.expanded;
                    if (h == Heuristic::Octile) totalExpandedBest += r.expanded;
                }
            } else {
                for (Heuristic h : hs4) {
                    auto r = astar(g, {0, 0}, {R - 1, C - 1}, moves, h);
                    assert(r.cost == ref);
                    if (h == Heuristic::Zero) totalExpandedZero += r.expanded;
                    if (h == Heuristic::Manhattan) totalExpandedBest += r.expanded;
                }
            }
            if (ref < INF) ++solvable;
        }
    }
    cout << "  solvable instances: " << solvable << " / 80\n";
    cout << "  total expansions  Zero heuristic: " << totalExpandedZero
         << "   informed heuristic: " << totalExpandedBest << "\n";
    assert(totalExpandedBest <= totalExpandedZero);
    cout << "  all costs match Dijkstra\n  OK\n\n";
}

void testHeuristicComparison() {
    cout << "=== Test 8: heuristic comparison on an open 30x30 grid ===\n";
    Grid g(vector<string>(30, string(30, '.')));
    pair<int, int> s{0, 0}, t{29, 17};
    for (Heuristic h : {Heuristic::Zero, Heuristic::Manhattan, Heuristic::Chebyshev,
                        Heuristic::Octile, Heuristic::Euclid}) {
        auto r = astar(g, s, t, MOVES8, h);
        cout << "  " << left << setw(16) << heuristicName(h) << " cost=" << setw(5) << r.cost
             << " expanded=" << r.expanded << "\n";
    }
    // Manhattan over-estimates with diagonal moves => may be non-optimal, so
    // we only assert for admissible ones.
    ll opt = astar(g, s, t, MOVES8, Heuristic::Zero).cost;
    assert(astar(g, s, t, MOVES8, Heuristic::Octile).cost == opt);
    assert(astar(g, s, t, MOVES8, Heuristic::Chebyshev).cost == opt);
    assert(astar(g, s, t, MOVES8, Heuristic::Euclid).cost == opt);
    // Expected optimum: 17 diagonals (14 each) + 12 straights (10 each).
    assert(opt == 17 * 14 + 12 * 10);
    cout << "  optimum = " << opt << " (17 diagonal + 12 straight moves)\n  OK\n\n";
}

int main() {
    testSmallGraph();
    testUnreachable();
    testRandomCrossCheck();
    testVariants();
    testAStarBasic();
    testAStarNoPath();
    testAStarVsDijkstra();
    testHeuristicComparison();
    cout << "All Dijkstra / A* tests passed.\n";
    return 0;
}
