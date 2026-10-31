/*
 * ============================================================================
 *  File    : 06_advanced_dynamic_programming.cpp
 *  Topic   : Advanced Dynamic Programming
 *  Standard: C++17
 *  Build   : g++ -std=c++17 -Wall -Wextra 06_advanced_dynamic_programming.cpp -o adp
 * ----------------------------------------------------------------------------
 *  Contents
 *    Part A - Knapsack variants
 *        A1. 0/1 knapsack (1-D rolling array) + item reconstruction (2-D)
 *        A2. Unbounded knapsack
 *        A3. Bounded knapsack (binary splitting of item counts)
 *        A4. Subset-sum counting and coin-change (ways / minimum coins)
 *    Part B - Bitmask DP
 *        B1. Travelling salesman (Held-Karp) O(2^n * n^2)
 *        B2. Minimum-cost assignment over subsets O(2^n * n)
 *        B3. Counting Hamiltonian paths
 *        B4. Subset-sum-over-subsets (SOS) DP
 *    Part C - Digit DP
 *        C1. Count numbers in [0,N] whose digit sum equals S
 *        C2. Count numbers in [1,N] with no two equal adjacent digits
 *        C3. Count numbers in [L,R] that do not contain digit 4
 *    Part D - Interval DP
 *        D1. Matrix-chain multiplication
 *        D2. Burst balloons
 *        D3. Longest palindromic subsequence
 *        D4. Minimum-cost merge of adjacent piles (stones)
 *
 *  Every DP is verified against a brute force on small inputs in main().
 * ============================================================================
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;
using ll = long long;

static const ll INF = (ll)4e18 / 4;

// Tiny deterministic random generator so that the output is reproducible.
struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed) {}
    int next(int mod) {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        return (int)((s >> 33) % (uint64_t)mod);
    }
};

// ============================================================================
// PART A: KNAPSACK VARIANTS
// ============================================================================

// ---- A1. 0/1 knapsack, space optimised -------------------------------------
// dp[c] = best value with capacity c. Iterating capacity downward guarantees
// each item is used at most once.
ll knapsack01(const vector<int>& w, const vector<int>& v, int cap) {
    vector<ll> dp(cap + 1, 0);
    for (size_t i = 0; i < w.size(); ++i)
        for (int c = cap; c >= w[i]; --c) dp[c] = max(dp[c], dp[c - w[i]] + v[i]);
    return dp[cap];
}

// 0/1 knapsack with reconstruction: full 2-D table, then walk backwards.
struct KnapsackResult {
    ll best;
    vector<int> chosen; // indices of chosen items
};

KnapsackResult knapsack01Reconstruct(const vector<int>& w, const vector<int>& v, int cap) {
    int n = (int)w.size();
    vector<vector<ll>> dp(n + 1, vector<ll>(cap + 1, 0));
    for (int i = 1; i <= n; ++i)
        for (int c = 0; c <= cap; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (c >= w[i - 1]) dp[i][c] = max(dp[i][c], dp[i - 1][c - w[i - 1]] + v[i - 1]);
        }
    KnapsackResult r{dp[n][cap], {}};
    int c = cap;
    for (int i = n; i >= 1; --i)
        if (dp[i][c] != dp[i - 1][c]) { // item i-1 was taken
            r.chosen.push_back(i - 1);
            c -= w[i - 1];
        }
    reverse(r.chosen.begin(), r.chosen.end());
    return r;
}

// ---- A2. Unbounded knapsack ------------------------------------------------
// Iterating capacity upward lets an item be reused arbitrarily often.
ll knapsackUnbounded(const vector<int>& w, const vector<int>& v, int cap) {
    vector<ll> dp(cap + 1, 0);
    for (int c = 0; c <= cap; ++c)
        for (size_t i = 0; i < w.size(); ++i)
            if (c >= w[i]) dp[c] = max(dp[c], dp[c - w[i]] + v[i]);
    return dp[cap];
}

// ---- A3. Bounded knapsack via binary splitting -----------------------------
// An item with count k is split into pieces of size 1,2,4,...,remainder.
// Any quantity 0..k is expressible as a sum of distinct pieces, so the problem
// reduces to 0/1 knapsack with O(log k) items per type.
ll knapsackBounded(const vector<int>& w, const vector<int>& v, const vector<int>& cnt, int cap) {
    vector<int> w2, v2;
    for (size_t i = 0; i < w.size(); ++i) {
        int k = cnt[i];
        for (int p = 1; k > 0; p <<= 1) {
            int take = min(p, k);
            w2.push_back(w[i] * take);
            v2.push_back(v[i] * take);
            k -= take;
        }
    }
    return knapsack01(w2, v2, cap);
}

// ---- A4. Counting problems -------------------------------------------------
// Number of subsets of a[] with sum exactly target.
ll countSubsetsWithSum(const vector<int>& a, int target) {
    vector<ll> dp(target + 1, 0);
    dp[0] = 1;
    for (int x : a)
        for (int s = target; s >= x; --s) dp[s] += dp[s - x];
    return dp[target];
}

// Number of ordered-insensitive ways (combinations) to make amount with coins.
ll coinWays(const vector<int>& coins, int amount) {
    vector<ll> dp(amount + 1, 0);
    dp[0] = 1;
    for (int c : coins) // coin loop outside => combinations, not permutations
        for (int s = c; s <= amount; ++s) dp[s] += dp[s - c];
    return dp[amount];
}

// Minimum number of coins; -1 if impossible.
int coinMin(const vector<int>& coins, int amount) {
    vector<int> dp(amount + 1, INT_MAX);
    dp[0] = 0;
    for (int s = 1; s <= amount; ++s)
        for (int c : coins)
            if (s >= c && dp[s - c] != INT_MAX) dp[s] = min(dp[s], dp[s - c] + 1);
    return dp[amount] == INT_MAX ? -1 : dp[amount];
}

// ---- brute forces for part A ----------------------------------------------
ll bruteKnapsack01(const vector<int>& w, const vector<int>& v, int cap) {
    int n = (int)w.size();
    ll best = 0;
    for (int m = 0; m < (1 << n); ++m) {
        ll tw = 0, tv = 0;
        for (int i = 0; i < n; ++i)
            if (m >> i & 1) tw += w[i], tv += v[i];
        if (tw <= cap) best = max(best, tv);
    }
    return best;
}

ll bruteBounded(const vector<int>& w, const vector<int>& v, const vector<int>& cnt, int cap,
                size_t i = 0) {
    if (i == w.size()) return 0;
    ll best = 0;
    for (int k = 0; k <= cnt[i] && k * w[i] <= cap; ++k)
        best = max(best, k * (ll)v[i] + bruteBounded(w, v, cnt, cap - k * w[i], i + 1));
    return best;
}

// ============================================================================
// PART B: BITMASK DP
// ============================================================================

// ---- B1. Held-Karp TSP -----------------------------------------------------
// dp[mask][j] = shortest path starting at city 0, visiting exactly the set
// `mask` and ending at j (j in mask). Answer closes the tour back to 0.
ll tsp(const vector<vector<ll>>& d) {
    int n = (int)d.size();
    if (n == 1) return 0;
    vector<vector<ll>> dp(1 << n, vector<ll>(n, INF));
    dp[1][0] = 0;
    for (int mask = 1; mask < (1 << n); mask += 2) { // only masks containing city 0
        for (int j = 0; j < n; ++j) {
            if (!(mask >> j & 1) || dp[mask][j] >= INF) continue;
            for (int k = 0; k < n; ++k) {
                if (mask >> k & 1) continue;
                int nm = mask | (1 << k);
                dp[nm][k] = min(dp[nm][k], dp[mask][j] + d[j][k]);
            }
        }
    }
    ll best = INF;
    for (int j = 1; j < n; ++j) best = min(best, dp[(1 << n) - 1][j] + d[j][0]);
    return best;
}

ll bruteTsp(const vector<vector<ll>>& d) {
    int n = (int)d.size();
    vector<int> perm(n - 1);
    iota(perm.begin(), perm.end(), 1);
    ll best = INF;
    do {
        ll len = 0;
        int cur = 0;
        for (int x : perm) len += d[cur][x], cur = x;
        len += d[cur][0];
        best = min(best, len);
    } while (next_permutation(perm.begin(), perm.end()));
    return n == 1 ? 0 : best;
}

// ---- B2. Assignment problem ------------------------------------------------
// dp[mask] = min cost where the first popcount(mask) workers have been given
// the jobs in mask. Worker i = popcount(mask) takes the next job.
ll assignment(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    vector<ll> dp(1 << n, INF);
    dp[0] = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        if (dp[mask] >= INF) continue;
        int worker = __builtin_popcount(mask);
        if (worker >= n) continue;
        for (int job = 0; job < n; ++job)
            if (!(mask >> job & 1))
                dp[mask | (1 << job)] = min(dp[mask | (1 << job)], dp[mask] + cost[worker][job]);
    }
    return dp[(1 << n) - 1];
}

ll bruteAssignment(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    ll best = INF;
    do {
        ll s = 0;
        for (int i = 0; i < n; ++i) s += cost[i][p[i]];
        best = min(best, s);
    } while (next_permutation(p.begin(), p.end()));
    return best;
}

// ---- B3. Counting Hamiltonian paths ---------------------------------------
// ways[mask][j] = number of simple paths that visit exactly `mask` and end at j.
ll countHamiltonianPaths(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<vector<ll>> ways(1 << n, vector<ll>(n, 0));
    for (int i = 0; i < n; ++i) ways[1 << i][i] = 1;
    for (int mask = 1; mask < (1 << n); ++mask)
        for (int j = 0; j < n; ++j) {
            if (!(mask >> j & 1) || !ways[mask][j]) continue;
            for (int k = 0; k < n; ++k)
                if (adj[j][k] && !(mask >> k & 1)) ways[mask | (1 << k)][k] += ways[mask][j];
        }
    ll total = 0;
    for (int j = 0; j < n; ++j) total += ways[(1 << n) - 1][j];
    return total;
}

ll bruteHamiltonian(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    ll cnt = 0;
    do {
        bool ok = true;
        for (int i = 0; i + 1 < n && ok; ++i) ok = adj[p[i]][p[i + 1]];
        cnt += ok;
    } while (next_permutation(p.begin(), p.end()));
    return cnt;
}

// ---- B4. Sum over subsets (SOS DP) ----------------------------------------
// F[mask] = sum of a[sub] over every sub that is a subset of mask. O(n 2^n).
vector<ll> sumOverSubsets(vector<ll> a, int bits) {
    for (int b = 0; b < bits; ++b)
        for (int mask = 0; mask < (1 << bits); ++mask)
            if (mask >> b & 1) a[mask] += a[mask ^ (1 << b)];
    return a;
}

// ============================================================================
// PART C: DIGIT DP
// ============================================================================
// General idea: process digits of N from most significant to least, tracking
// a "tight" flag (prefix equals N's prefix so far) plus problem-specific state.
// Memoise only on non-tight states.

// ---- C1. Digit sum equals S -----------------------------------------------
ll countDigitSum(ll N, int S) {
    if (N < 0) return 0;
    string d = to_string(N);
    int len = (int)d.size();
    // memo[pos][sum] for non-tight states; -1 = not computed.
    vector<vector<ll>> memo(len + 1, vector<ll>(S + 1, -1));
    function<ll(int, int, bool)> go = [&](int pos, int sum, bool tight) -> ll {
        if (sum > S) return 0;
        if (pos == len) return sum == S ? 1 : 0;
        if (!tight && memo[pos][sum] != -1) return memo[pos][sum];
        int lim = tight ? d[pos] - '0' : 9;
        ll res = 0;
        for (int x = 0; x <= lim; ++x) res += go(pos + 1, sum + x, tight && x == lim);
        if (!tight) memo[pos][sum] = res;
        return res;
    };
    return go(0, 0, true);
}

// ---- C2. No two equal adjacent digits (numbers 1..N) ----------------------
// State: previous digit (10 = none yet) and `started` flag so that leading
// zeros do not count as digits.
ll countNoAdjacentEqual(ll N) {
    if (N <= 0) return 0;
    string d = to_string(N);
    int len = (int)d.size();
    // memo[pos][prev][started] for non-tight states
    vector<vector<array<ll, 2>>> memo(len + 1, vector<array<ll, 2>>(11, {-1, -1}));
    function<ll(int, int, bool, bool)> go = [&](int pos, int prev, bool tight, bool started) -> ll {
        if (pos == len) return started ? 1 : 0; // exclude the number 0
        if (!tight && memo[pos][prev][started] != -1) return memo[pos][prev][started];
        int lim = tight ? d[pos] - '0' : 9;
        ll res = 0;
        for (int x = 0; x <= lim; ++x) {
            bool nt = tight && x == lim;
            if (!started && x == 0) {
                res += go(pos + 1, 10, nt, false); // still in leading zeros
            } else if (started && x == prev) {
                continue; // equal adjacent digits forbidden
            } else {
                res += go(pos + 1, x, nt, true);
            }
        }
        if (!tight) memo[pos][prev][started] = res;
        return res;
    };
    return go(0, 10, true, false);
}

// ---- C3. Numbers in [0,N] without the digit 4 ------------------------------
ll countNo4UpTo(ll N) {
    if (N < 0) return 0;
    string d = to_string(N);
    int len = (int)d.size();
    // Closed form via digit DP: count numbers (with leading zeros allowed)
    // where no digit equals 4. Leading zeros are fine because 0 != 4.
    ll res = 0;
    ll free9 = 1; // 9^(remaining positions)
    vector<ll> pw(len + 1, 1);
    for (int i = 1; i <= len; ++i) pw[i] = pw[i - 1] * 9;
    (void)free9;
    for (int pos = 0; pos < len; ++pos) {
        int cur = d[pos] - '0';
        for (int x = 0; x < cur; ++x)
            if (x != 4) res += pw[len - pos - 1];
        if (cur == 4) return res; // N's own prefix now contains 4, N excluded
    }
    return res + 1; // N itself has no 4
}

ll countNo4Range(ll L, ll R) { return countNo4UpTo(R) - countNo4UpTo(L - 1); }

static int digitSum(ll x) {
    int s = 0;
    for (; x; x /= 10) s += (int)(x % 10);
    return s;
}
static bool noAdjEq(ll x) {
    int prev = -1;
    for (; x; x /= 10) {
        int dg = (int)(x % 10);
        if (dg == prev) return false;
        prev = dg;
    }
    return true;
}
static bool has4(ll x) {
    for (; x; x /= 10)
        if (x % 10 == 4) return true;
    return false;
}

// ============================================================================
// PART D: INTERVAL DP
// ============================================================================

// ---- D1. Matrix chain multiplication --------------------------------------
// Matrix i has dimensions p[i] x p[i+1]. dp[i][j] = min scalar mults for i..j.
ll matrixChain(const vector<int>& p, vector<vector<int>>* splitOut = nullptr) {
    int n = (int)p.size() - 1;
    vector<vector<ll>> dp(n, vector<ll>(n, 0));
    vector<vector<int>> split(n, vector<int>(n, 0));
    for (int len = 2; len <= n; ++len)
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;
            for (int k = i; k < j; ++k) {
                ll c = dp[i][k] + dp[k + 1][j] + (ll)p[i] * p[k + 1] * p[j + 1];
                if (c < dp[i][j]) dp[i][j] = c, split[i][j] = k;
            }
        }
    if (splitOut) *splitOut = split;
    return dp[0][n - 1];
}

string parenthesize(const vector<vector<int>>& split, int i, int j) {
    if (i == j) return string(1, (char)('A' + i));
    int k = split[i][j];
    return "(" + parenthesize(split, i, k) + parenthesize(split, k + 1, j) + ")";
}

// ---- D2. Burst balloons ----------------------------------------------------
// dp[l][r] = max coins from bursting all balloons strictly between l and r;
// k is the LAST balloon burst in (l,r), so its neighbours are exactly l and r.
ll burstBalloons(vector<int> nums) {
    nums.insert(nums.begin(), 1);
    nums.push_back(1);
    int n = (int)nums.size();
    vector<vector<ll>> dp(n, vector<ll>(n, 0));
    for (int len = 2; len < n; ++len)
        for (int l = 0; l + len < n; ++l) {
            int r = l + len;
            for (int k = l + 1; k < r; ++k)
                dp[l][r] = max(dp[l][r], dp[l][k] + dp[k][r] + (ll)nums[l] * nums[k] * nums[r]);
        }
    return dp[0][n - 1];
}

ll bruteBurst(vector<int> nums) {
    if (nums.empty()) return 0;
    ll best = 0;
    for (size_t i = 0; i < nums.size(); ++i) {
        ll left = i ? nums[i - 1] : 1, right = i + 1 < nums.size() ? nums[i + 1] : 1;
        ll gain = left * nums[i] * right;
        vector<int> rest = nums;
        rest.erase(rest.begin() + (long)i);
        best = max(best, gain + bruteBurst(rest));
    }
    return best;
}

// ---- D3. Longest palindromic subsequence ----------------------------------
int longestPalSubseq(const string& s) {
    int n = (int)s.size();
    if (n == 0) return 0;
    vector<vector<int>> dp(n, vector<int>(n, 0));
    for (int i = n - 1; i >= 0; --i) {
        dp[i][i] = 1;
        for (int j = i + 1; j < n; ++j)
            dp[i][j] = s[i] == s[j] ? dp[i + 1][j - 1] + 2 : max(dp[i + 1][j], dp[i][j - 1]);
    }
    return dp[0][n - 1];
}

int brutePalSubseq(const string& s) {
    int n = (int)s.size(), best = 0;
    for (int m = 1; m < (1 << n); ++m) {
        string t;
        for (int i = 0; i < n; ++i)
            if (m >> i & 1) t += s[i];
        string r(t.rbegin(), t.rend());
        if (t == r) best = max(best, (int)t.size());
    }
    return best;
}

// ---- D4. Merge adjacent piles ----------------------------------------------
// Merging two adjacent piles costs the sum of their sizes. Minimise total.
ll mergePiles(const vector<int>& a) {
    int n = (int)a.size();
    vector<ll> pre(n + 1, 0);
    for (int i = 0; i < n; ++i) pre[i + 1] = pre[i] + a[i];
    vector<vector<ll>> dp(n, vector<ll>(n, 0));
    for (int len = 2; len <= n; ++len)
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;
            for (int k = i; k < j; ++k) dp[i][j] = min(dp[i][j], dp[i][k] + dp[k + 1][j]);
            dp[i][j] += pre[j + 1] - pre[i];
        }
    return n ? dp[0][n - 1] : 0;
}

// Brute force: try every merge order recursively.
ll bruteMerge(vector<int> a) {
    if (a.size() <= 1) return 0;
    ll best = INF;
    for (size_t i = 0; i + 1 < a.size(); ++i) {
        vector<int> b = a;
        int s = a[i] + a[i + 1];
        b[i] = s;
        b.erase(b.begin() + (long)i + 1);
        best = min(best, s + bruteMerge(b));
    }
    return best;
}

// ============================================================================
// TESTS
// ============================================================================

static void check(bool ok, const string& what) {
    cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << what << "\n";
    assert(ok);
}

void testKnapsack() {
    cout << "=== Part A: knapsack variants ===\n";
    vector<int> w = {1, 3, 4, 5}, v = {1, 4, 5, 7};
    cout << "  classic 0/1 (cap 7): " << knapsack01(w, v, 7) << "\n";
    check(knapsack01(w, v, 7) == 9, "0/1 knapsack classic = 9");
    auto r = knapsack01Reconstruct(w, v, 7);
    cout << "  chosen items:";
    ll tw = 0, tv = 0;
    for (int i : r.chosen) cout << " " << i, tw += w[i], tv += v[i];
    cout << "  (weight " << tw << ", value " << tv << ")\n";
    check(tv == r.best && tw <= 7, "reconstruction consistent");

    Rng rng(42);
    bool ok = true;
    for (int t = 0; t < 200 && ok; ++t) {
        int n = 1 + rng.next(10);
        vector<int> ww(n), vv(n);
        for (int i = 0; i < n; ++i) ww[i] = 1 + rng.next(9), vv[i] = 1 + rng.next(20);
        int cap = rng.next(30);
        ok = knapsack01(ww, vv, cap) == bruteKnapsack01(ww, vv, cap) &&
             knapsack01Reconstruct(ww, vv, cap).best == bruteKnapsack01(ww, vv, cap);
    }
    check(ok, "200 random 0/1 instances match brute force");

    check(knapsackUnbounded({2, 3, 7}, {5, 7, 20}, 14) == 40, "unbounded: cap 14 -> two 7s = 40");
    check(knapsackUnbounded({5}, {10}, 4) == 0, "unbounded: nothing fits = 0");

    ok = true;
    for (int t = 0; t < 200 && ok; ++t) {
        int n = 1 + rng.next(4);
        vector<int> ww(n), vv(n), cc(n);
        for (int i = 0; i < n; ++i) ww[i] = 1 + rng.next(6), vv[i] = 1 + rng.next(15), cc[i] = 1 + rng.next(6);
        int cap = rng.next(30);
        ok = knapsackBounded(ww, vv, cc, cap) == bruteBounded(ww, vv, cc, cap);
    }
    check(ok, "200 random bounded instances match brute force");

    check(countSubsetsWithSum({1, 2, 3, 4, 5}, 5) == 3, "subsets of {1..5} with sum 5 = 3 ({5},{1,4},{2,3})");
    check(coinWays({1, 2, 5}, 5) == 4, "coin ways for 5 with {1,2,5} = 4");
    check(coinMin({1, 5, 10, 25}, 63) == 6, "min coins for 63 = 6 (25+25+10+1+1+1)");
    check(coinMin({2}, 3) == -1, "min coins impossible = -1");
    cout << "\n";
}

void testBitmask() {
    cout << "=== Part B: bitmask DP ===\n";
    vector<vector<ll>> d = {{0, 10, 15, 20}, {10, 0, 35, 25}, {15, 35, 0, 30}, {20, 25, 30, 0}};
    cout << "  TSP 4 cities: " << tsp(d) << "\n";
    check(tsp(d) == 80, "classic 4-city TSP = 80");

    Rng rng(7);
    bool ok = true;
    for (int t = 0; t < 30 && ok; ++t) {
        int n = 2 + rng.next(6);
        vector<vector<ll>> m(n, vector<ll>(n, 0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (i != j) m[i][j] = 1 + rng.next(50);
        ok = tsp(m) == bruteTsp(m);
    }
    check(ok, "30 random TSP instances match brute force");

    ok = true;
    for (int t = 0; t < 30 && ok; ++t) {
        int n = 1 + rng.next(7);
        vector<vector<int>> c(n, vector<int>(n));
        for (auto& row : c)
            for (int& x : row) x = rng.next(100);
        ok = assignment(c) == bruteAssignment(c);
    }
    check(ok, "30 random assignment instances match brute force");

    ok = true;
    for (int t = 0; t < 30 && ok; ++t) {
        int n = 1 + rng.next(7);
        vector<vector<int>> adj(n, vector<int>(n, 0));
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) adj[i][j] = adj[j][i] = rng.next(100) < 60;
        ok = countHamiltonianPaths(adj) == bruteHamiltonian(adj);
    }
    check(ok, "30 random graphs: Hamiltonian path counts match brute force");

    // Complete graph K5 has 5! = 120 directed Hamiltonian paths.
    vector<vector<int>> k5(5, vector<int>(5, 1));
    for (int i = 0; i < 5; ++i) k5[i][i] = 0;
    check(countHamiltonianPaths(k5) == 120, "K5 has 120 Hamiltonian paths");

    int bits = 5;
    vector<ll> a(1 << bits);
    for (auto& x : a) x = rng.next(10);
    auto F = sumOverSubsets(a, bits);
    ok = true;
    for (int mask = 0; mask < (1 << bits); ++mask) {
        ll s = 0;
        for (int sub = 0; sub < (1 << bits); ++sub)
            if ((sub & mask) == sub) s += a[sub];
        ok = ok && s == F[mask];
    }
    check(ok, "SOS DP matches naive subset enumeration (5 bits)");
    cout << "\n";
}

void testDigitDP() {
    cout << "=== Part C: digit DP ===\n";
    cout << "  numbers in [0,1000000] with digit sum 20: " << countDigitSum(1000000, 20) << "\n";
    bool ok = true;
    for (ll N : {0LL, 9LL, 10LL, 99LL, 100LL, 1234LL, 20000LL}) {
        for (int S : {0, 1, 5, 9, 13, 27}) {
            ll bf = 0;
            for (ll x = 0; x <= N; ++x) bf += digitSum(x) == S;
            ok = ok && countDigitSum(N, S) == bf;
        }
    }
    check(ok, "digit-sum DP matches brute force");

    ok = true;
    for (ll N : {1LL, 9LL, 10LL, 11LL, 100LL, 999LL, 12345LL, 99999LL}) {
        ll bf = 0;
        for (ll x = 1; x <= N; ++x) bf += noAdjEq(x);
        ok = ok && countNoAdjacentEqual(N) == bf;
    }
    cout << "  no-adjacent-equal in [1,10^6]: " << countNoAdjacentEqual(1000000) << "\n";
    check(ok, "no-adjacent-equal DP matches brute force");

    ok = true;
    for (auto [L, R] : vector<pair<ll, ll>>{{1, 100}, {40, 50}, {1, 99999}, {444, 4444}, {0, 0}}) {
        ll bf = 0;
        for (ll x = L; x <= R; ++x) bf += !has4(x);
        ok = ok && countNo4Range(L, R) == bf;
    }
    cout << "  numbers in [1,100] without a 4: " << countNo4Range(1, 100) << "\n";
    check(ok, "no-digit-4 count matches brute force");
    cout << "\n";
}

void testInterval() {
    cout << "=== Part D: interval DP ===\n";
    vector<int> p = {40, 20, 30, 10, 30};
    vector<vector<int>> split;
    ll mc = matrixChain(p, &split);
    cout << "  matrix chain " << mc << " via " << parenthesize(split, 0, 3) << "\n";
    check(mc == 26000, "matrix chain (40,20,30,10,30) = 26000");
    check(matrixChain({10, 20, 30}) == 6000, "two matrices: 10*20*30 = 6000");

    check(burstBalloons({3, 1, 5, 8}) == 167, "burst balloons {3,1,5,8} = 167");
    Rng rng(99);
    bool ok = true;
    for (int t = 0; t < 40 && ok; ++t) {
        int n = rng.next(8);
        vector<int> a(n);
        for (int& x : a) x = 1 + rng.next(9);
        ok = burstBalloons(a) == bruteBurst(a);
    }
    check(ok, "40 random balloon instances match brute force");

    check(longestPalSubseq("bbbab") == 4, "LPS(bbbab) = 4");
    check(longestPalSubseq("character") == 5, "LPS(character) = 5");
    ok = true;
    for (int t = 0; t < 60 && ok; ++t) {
        int n = rng.next(13);
        string s;
        for (int i = 0; i < n; ++i) s += (char)('a' + rng.next(3));
        ok = longestPalSubseq(s) == brutePalSubseq(s);
    }
    check(ok, "60 random strings: LPS matches brute force");

    check(mergePiles({3, 4, 5}) == 19, "merge piles {3,4,5} = 19");
    ok = true;
    for (int t = 0; t < 40 && ok; ++t) {
        int n = 1 + rng.next(7);
        vector<int> a(n);
        for (int& x : a) x = 1 + rng.next(20);
        ok = mergePiles(a) == bruteMerge(a);
    }
    check(ok, "40 random pile instances match brute force");
    cout << "\n";
}

int main() {
    testKnapsack();
    testBitmask();
    testDigitDP();
    testInterval();
    cout << "All advanced DP tests passed.\n";
    return 0;
}
