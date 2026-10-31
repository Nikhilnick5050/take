/*
 * ============================================================================
 *  02_segment_tree_lazy.cpp
 * ============================================================================
 *  Segment tree with lazy propagation.
 *
 *  Supported operations (all O(log n)):
 *    - range_add(l, r, v) : add v to every element in [l, r]
 *    - range_sum(l, r)    : sum of elements in [l, r]
 *    - range_min(l, r)    : minimum of elements in [l, r]
 *    - range_max(l, r)    : maximum of elements in [l, r]
 *    - point_get(i)       : value at one position
 *    - find_first_leq(l,x): first index >= l with value <= x (tree descent)
 *
 *  Idea of lazy propagation:
 *    Each node stores aggregates (sum, min, max) of its segment *as if all
 *    pending additions above it were already applied to it*, plus a "lazy"
 *    tag meaning "add this to every element of both children, not yet pushed
 *    down". Updates that fully cover a node only touch that node and set its
 *    tag; the tag is pushed to the children only when we need to descend.
 *
 *  Two implementations are provided:
 *    1. LazySegTree      - the efficient structure (array based, 1-indexed)
 *    2. NaiveArray       - O(n) per operation reference used for verification
 *
 *  Build:  g++ -std=c++17 -Wall -Wextra -O2 02_segment_tree_lazy.cpp -o seg
 * ============================================================================
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using i64 = long long;

// ---------------------------------------------------------------------------
//  LazySegTree
// ---------------------------------------------------------------------------
class LazySegTree {
public:
    static constexpr i64 INF = std::numeric_limits<i64>::max() / 4;

    // Build from an initial array in O(n).
    explicit LazySegTree(const std::vector<i64>& init)
        : n_(static_cast<int>(init.size())) {
        int cap = 1;
        while (cap < std::max(1, n_)) cap <<= 1;
        sum_.assign(4 * static_cast<size_t>(std::max(1, n_)), 0);
        mn_.assign(sum_.size(), INF);
        mx_.assign(sum_.size(), -INF);
        lazy_.assign(sum_.size(), 0);
        if (n_ > 0) build(1, 0, n_ - 1, init);
    }

    // Convenience: n zeros.
    explicit LazySegTree(int n) : LazySegTree(std::vector<i64>(n, 0)) {}

    int size() const { return n_; }

    // Add v to all elements in the inclusive range [l, r].
    void range_add(int l, int r, i64 v) {
        if (!valid(l, r)) return;
        add(1, 0, n_ - 1, l, r, v);
    }

    // Sum of all elements in [l, r].
    i64 range_sum(int l, int r) {
        if (!valid(l, r)) return 0;
        return query_sum(1, 0, n_ - 1, l, r);
    }

    // Minimum in [l, r].
    i64 range_min(int l, int r) {
        if (!valid(l, r)) return INF;
        return query_min(1, 0, n_ - 1, l, r);
    }

    // Maximum in [l, r].
    i64 range_max(int l, int r) {
        if (!valid(l, r)) return -INF;
        return query_max(1, 0, n_ - 1, l, r);
    }

    i64 point_get(int i) { return range_sum(i, i); }

    // First index p >= from with a[p] <= x, or -1 if none exists.
    // Uses the stored minima to prune whole subtrees: O(log n).
    int find_first_leq(int from, i64 x) {
        if (n_ == 0 || from >= n_) return -1;
        if (from < 0) from = 0;
        return first_leq(1, 0, n_ - 1, from, x);
    }

    // Materialise the whole array (pushes everything down).
    std::vector<i64> to_vector() {
        std::vector<i64> out(n_);
        for (int i = 0; i < n_; ++i) out[i] = point_get(i);
        return out;
    }

private:
    int n_;
    // For node p covering a segment of length len:
    //   sum_[p] : true sum of the segment (excluding tags of ancestors)
    //   mn_[p]  : true minimum   (same convention)
    //   mx_[p]  : true maximum
    //   lazy_[p]: pending addition for the children
    std::vector<i64> sum_, mn_, mx_, lazy_;

    bool valid(int l, int r) const { return n_ > 0 && l <= r && l < n_ && r >= 0; }

    // Recompute node p from its children.
    void pull(int p) {
        sum_[p] = sum_[2 * p] + sum_[2 * p + 1];
        mn_[p] = std::min(mn_[2 * p], mn_[2 * p + 1]);
        mx_[p] = std::max(mx_[2 * p], mx_[2 * p + 1]);
    }

    // Apply "add v to every element" to node p covering `len` elements.
    void apply(int p, int len, i64 v) {
        sum_[p] += v * len;
        mn_[p] += v;
        mx_[p] += v;
        lazy_[p] += v;
    }

    // Move the pending tag of p down to its two children.
    void push(int p, int lo, int hi) {
        if (lazy_[p] == 0) return;
        int mid = lo + (hi - lo) / 2;
        apply(2 * p, mid - lo + 1, lazy_[p]);
        apply(2 * p + 1, hi - mid, lazy_[p]);
        lazy_[p] = 0;
    }

    void build(int p, int lo, int hi, const std::vector<i64>& a) {
        if (lo == hi) {
            sum_[p] = mn_[p] = mx_[p] = a[lo];
            return;
        }
        int mid = lo + (hi - lo) / 2;
        build(2 * p, lo, mid, a);
        build(2 * p + 1, mid + 1, hi, a);
        pull(p);
    }

    void add(int p, int lo, int hi, int l, int r, i64 v) {
        if (r < lo || hi < l) return;                 // disjoint
        if (l <= lo && hi <= r) {                     // fully covered
            apply(p, hi - lo + 1, v);
            return;
        }
        push(p, lo, hi);                              // partial: go down
        int mid = lo + (hi - lo) / 2;
        add(2 * p, lo, mid, l, r, v);
        add(2 * p + 1, mid + 1, hi, l, r, v);
        pull(p);
    }

    i64 query_sum(int p, int lo, int hi, int l, int r) {
        if (r < lo || hi < l) return 0;
        if (l <= lo && hi <= r) return sum_[p];
        push(p, lo, hi);
        int mid = lo + (hi - lo) / 2;
        return query_sum(2 * p, lo, mid, l, r) +
               query_sum(2 * p + 1, mid + 1, hi, l, r);
    }

    i64 query_min(int p, int lo, int hi, int l, int r) {
        if (r < lo || hi < l) return INF;
        if (l <= lo && hi <= r) return mn_[p];
        push(p, lo, hi);
        int mid = lo + (hi - lo) / 2;
        return std::min(query_min(2 * p, lo, mid, l, r),
                        query_min(2 * p + 1, mid + 1, hi, l, r));
    }

    i64 query_max(int p, int lo, int hi, int l, int r) {
        if (r < lo || hi < l) return -INF;
        if (l <= lo && hi <= r) return mx_[p];
        push(p, lo, hi);
        int mid = lo + (hi - lo) / 2;
        return std::max(query_max(2 * p, lo, mid, l, r),
                        query_max(2 * p + 1, mid + 1, hi, l, r));
    }

    int first_leq(int p, int lo, int hi, int from, i64 x) {
        if (hi < from || mn_[p] > x) return -1;  // nothing useful here
        if (lo == hi) return lo;
        push(p, lo, hi);
        int mid = lo + (hi - lo) / 2;
        int res = first_leq(2 * p, lo, mid, from, x);
        if (res != -1) return res;
        return first_leq(2 * p + 1, mid + 1, hi, from, x);
    }
};

// ---------------------------------------------------------------------------
//  NaiveArray: trivially correct O(n) reference implementation
// ---------------------------------------------------------------------------
class NaiveArray {
public:
    explicit NaiveArray(std::vector<i64> a) : a_(std::move(a)) {}

    void range_add(int l, int r, i64 v) {
        for (int i = std::max(l, 0); i <= r && i < (int)a_.size(); ++i) a_[i] += v;
    }
    i64 range_sum(int l, int r) const {
        i64 s = 0;
        for (int i = std::max(l, 0); i <= r && i < (int)a_.size(); ++i) s += a_[i];
        return s;
    }
    i64 range_min(int l, int r) const {
        i64 m = LazySegTree::INF;
        for (int i = std::max(l, 0); i <= r && i < (int)a_.size(); ++i)
            m = std::min(m, a_[i]);
        return m;
    }
    i64 range_max(int l, int r) const {
        i64 m = -LazySegTree::INF;
        for (int i = std::max(l, 0); i <= r && i < (int)a_.size(); ++i)
            m = std::max(m, a_[i]);
        return m;
    }
    int find_first_leq(int from, i64 x) const {
        for (int i = std::max(from, 0); i < (int)a_.size(); ++i)
            if (a_[i] <= x) return i;
        return -1;
    }
    const std::vector<i64>& data() const { return a_; }

private:
    std::vector<i64> a_;
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

static std::string show(const std::vector<i64>& v) {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) os << (i ? ", " : "") << v[i];
    os << "]";
    return os.str();
}

// ---------------------------------------------------------------------------
//  Tests
// ---------------------------------------------------------------------------
static void testSmallExample() {
    std::cout << "== Small worked example ==\n";
    //            index: 0  1  2  3  4  5  6  7
    std::vector<i64> a = {5, 3, 8, 1, 9, 2, 7, 4};
    LazySegTree st(a);
    std::cout << "  initial         : " << show(st.to_vector()) << "\n";

    CHECK(st.range_sum(0, 7) == 39);
    CHECK(st.range_min(0, 7) == 1);
    CHECK(st.range_max(0, 7) == 9);
    CHECK(st.range_sum(2, 4) == 18);
    CHECK(st.range_min(4, 6) == 2);
    std::cout << "  sum[0,7]=" << st.range_sum(0, 7)
              << " min[0,7]=" << st.range_min(0, 7)
              << " max[0,7]=" << st.range_max(0, 7) << "\n";

    st.range_add(2, 5, 10);  // 5 3 18 11 19 12 7 4
    std::cout << "  add(2,5,+10)    : " << show(st.to_vector()) << "\n";
    CHECK(st.range_sum(0, 7) == 39 + 40);
    CHECK(st.range_min(2, 5) == 11);
    CHECK(st.range_max(0, 7) == 19);
    CHECK(st.point_get(3) == 11);

    st.range_add(0, 3, -4);  // 1 -1 14 7 19 12 7 4
    std::cout << "  add(0,3,-4)     : " << show(st.to_vector()) << "\n";
    CHECK(st.range_min(0, 7) == -1);
    CHECK(st.range_sum(0, 3) == 21);
    CHECK(st.range_sum(0, 7) == 63);

    // Overlapping updates exercise tag composition.
    st.range_add(1, 6, 100);
    st.range_add(3, 4, -50);
    std::cout << "  more updates    : " << show(st.to_vector()) << "\n";
    CHECK(st.range_min(1, 1) == 99);
    CHECK(st.range_min(3, 4) == 57);
}

static void testEdgeCases() {
    std::cout << "== Edge cases ==\n";
    // Size 1.
    LazySegTree one(std::vector<i64>{42});
    one.range_add(0, 0, 8);
    CHECK(one.range_sum(0, 0) == 50);
    CHECK(one.range_min(0, 0) == 50);
    CHECK(one.range_max(0, 0) == 50);

    // Empty tree: queries return neutral elements.
    LazySegTree empty(std::vector<i64>{});
    CHECK(empty.range_sum(0, 0) == 0);
    CHECK(empty.range_min(0, 0) == LazySegTree::INF);
    empty.range_add(0, 0, 5);  // must not crash
    CHECK(empty.size() == 0);

    // Out-of-range / inverted queries are clamped or ignored.
    LazySegTree s(5);
    s.range_add(3, 100, 7);  // clamps to [3,4]
    CHECK(s.range_sum(0, 4) == 14);
    CHECK(s.range_sum(4, 3) == 0);

    // Negative values and large magnitudes.
    LazySegTree big(std::vector<i64>{-1000000000000LL, 1000000000000LL, 0});
    big.range_add(0, 2, 1000000000000LL);
    CHECK(big.range_sum(0, 2) == 3000000000000LL);
    CHECK(big.range_min(0, 2) == 0);
    std::cout << "  edge cases done\n";
}

static void testFindFirst() {
    std::cout << "== find_first_leq (tree descent) ==\n";
    LazySegTree st(std::vector<i64>{9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
    CHECK(st.find_first_leq(0, 5) == 4);
    CHECK(st.find_first_leq(5, 5) == 5);
    CHECK(st.find_first_leq(0, -1) == -1);
    st.range_add(4, 9, 100);  // indices 4..9 become large
    CHECK(st.find_first_leq(0, 5) == -1);
    CHECK(st.find_first_leq(0, 6) == 3);
    st.range_add(0, 9, -100);
    // values are now -91 -92 -93 -94 -95 ... so the first <= -93 is index 2
    CHECK(st.find_first_leq(0, -93) == 2);
    CHECK(st.find_first_leq(3, -93) == 3);
    std::cout << "  first<=-93 at   : " << st.find_first_leq(0, -93) << "\n";
}

static void testRandomized(int n, int ops, unsigned seed) {
    std::cout << "== Randomized vs naive: n=" << n << " ops=" << ops
              << " seed=" << seed << " ==\n";
    std::mt19937 rng(seed);
    auto rnd = [&](int lo, int hi) {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    };

    std::vector<i64> init(n);
    for (auto& x : init) x = rnd(-1000, 1000);
    LazySegTree st(init);
    NaiveArray ref(init);

    int mismatches = 0;
    int adds = 0, queries = 0;
    for (int i = 0; i < ops; ++i) {
        int l = rnd(0, n - 1), r = rnd(0, n - 1);
        if (l > r) std::swap(l, r);
        int type = rnd(0, 5);
        if (type <= 1) {
            i64 v = rnd(-500, 500);
            st.range_add(l, r, v);
            ref.range_add(l, r, v);
            ++adds;
        } else if (type == 2) {
            if (st.range_sum(l, r) != ref.range_sum(l, r)) ++mismatches;
            ++queries;
        } else if (type == 3) {
            if (st.range_min(l, r) != ref.range_min(l, r)) ++mismatches;
            ++queries;
        } else if (type == 4) {
            if (st.range_max(l, r) != ref.range_max(l, r)) ++mismatches;
            ++queries;
        } else {
            i64 x = rnd(-3000, 3000);
            if (st.find_first_leq(l, x) != ref.find_first_leq(l, x)) ++mismatches;
            ++queries;
        }
    }
    if (st.to_vector() != ref.data()) ++mismatches;
    CHECK(mismatches == 0);
    std::cout << "  updates=" << adds << " queries=" << queries
              << " mismatches=" << mismatches << "\n";
}

static void testPerformance() {
    std::cout << "== Larger workload (n=200000, 200000 ops) ==\n";
    const int n = 200000;
    std::mt19937 rng(99);
    std::vector<i64> a(n);
    for (int i = 0; i < n; ++i) a[i] = i % 1000;
    LazySegTree st(a);
    i64 checksum = 0;
    for (int i = 0; i < 200000; ++i) {
        int l = rng() % n, r = rng() % n;
        if (l > r) std::swap(l, r);
        if (i % 2 == 0) st.range_add(l, r, static_cast<i64>(rng() % 21) - 10);
        else checksum += st.range_sum(l, r) + st.range_min(l, r);
    }
    std::cout << "  checksum        : " << checksum << "\n";
    CHECK(st.size() == n);
}

int main() {
    std::cout << "Lazy segment tree: range add / range sum / range min\n\n";
    testSmallExample();
    testEdgeCases();
    testFindFirst();
    testRandomized(1, 500, 1);
    testRandomized(2, 500, 2);
    testRandomized(17, 5000, 3);
    testRandomized(100, 20000, 4);
    testRandomized(1000, 30000, 5);
    testPerformance();
    std::cout << "\n" << (g_failures == 0 ? "ALL TESTS PASSED"
                                          : "SOME TESTS FAILED")
              << " (failures: " << g_failures << ")\n";
    return g_failures == 0 ? 0 : 1;
}
