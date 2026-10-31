/*
 * ============================================================================
 *  03_trie_suffix_array.cpp    
 * ===================================vdav adb=========================================
 *  String data structures: Trie, Suffidv x Array and LCP array.
 *
 *  Part 1 - Trie (prefix tree)
 *    insert, contains, erase, count_with_prefix, autocomplete (lexicographic),
 *    longest_common_prefix of all stored words.
 *
 *  Part 2 - Suffix array (prefix doubling, O(n log n) with counting sort)
 *    sa[i]   : starting index of the i-th smallest suffix
 *    rank[i] : position of suffix i inside sa
 *
 *  Part 3 - LCP array (Kasai, O(n))
 *    lcp[i]  : longest common prefix of suffixes sa[i] and sa[i+1]
 *              (sized n-1; lcp[i] relates sorted neighbours i and i+1)
 *
 *  Applications built on top of them:
 *    - pattern search by binary search over the suffix array
 *    - number of occurrences of a pattern
 *    - number of distinct substrings = n(n+1)/2 - sum(lcp)
 *    - longest repeated substring = max(lcp)
 *    - longest common substring of two strings (separator trick)
 *    - LCP of arbitrary suffix pairs via a sparse table (range minimum)
 *
 *  Build:  g++ -std=c++17 -Wall -Wextra -O2 03_trie_suffix_array.cpp -o sa
 * ============================================================================
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// ===========================================================================
//  Part 1: Trie
// ===========================================================================
class Trie {
    struct Node {
        std::array<std::unique_ptr<Node>, 26> next{};  // lowercase a-z
        int pass = 0;      // number of stored words passing through this node
        bool end = false;  // a stored word ends exactly here
    };
    std::unique_ptr<Node> root_ = std::make_unique<Node>();
    int words_ = 0;

    static int idx(char c) { return c - 'a'; }
    static bool ok(const std::string& s) {
        return std::all_of(s.begin(), s.end(),
                           [](char c) { return c >= 'a' && c <= 'z'; });
    }

    // Node reached by following s, or nullptr.
    const Node* find(const std::string& s) const {
        const Node* cur = root_.get();
        for (char c : s) {
            if (c < 'a' || c > 'z') return nullptr;
            cur = cur->next[idx(c)].get();
            if (!cur) return nullptr;
        }
        return cur;
    }

    // Depth-first walk in alphabetical order, collecting words.
    void collect(const Node* n, std::string& cur, std::vector<std::string>& out,
                 size_t limit) const {
        if (out.size() >= limit) return;
        if (n->end) out.push_back(cur);
        for (int c = 0; c < 26; ++c) {
            if (n->next[c]) {
                cur.push_back(static_cast<char>('a' + c));
                collect(n->next[c].get(), cur, out, limit);
                cur.pop_back();
                if (out.size() >= limit) return;
            }
        }
    }

public:
    int size() const { return words_; }

    // Returns false if the word is already present or has invalid characters.
    bool insert(const std::string& s) {
        if (!ok(s) || contains(s)) return false;
        Node* cur = root_.get();
        ++cur->pass;
        for (char c : s) {
            auto& child = cur->next[idx(c)];
            if (!child) child = std::make_unique<Node>();
            cur = child.get();
            ++cur->pass;
        }
        cur->end = true;
        ++words_;
        return true;
    }

    bool contains(const std::string& s) const {
        const Node* n = find(s);
        return n && n->end;
    }

    // Number of stored words that start with prefix.
    int count_with_prefix(const std::string& prefix) const {
        const Node* n = find(prefix);
        return n ? n->pass : 0;
    }

    // Remove a word; prunes nodes that no longer lead to any word.
    bool erase(const std::string& s) {
        if (!contains(s)) return false;
        Node* cur = root_.get();
        --cur->pass;
        for (char c : s) {
            auto& child = cur->next[idx(c)];
            Node* nxt = child.get();
            if (--nxt->pass == 0) {   // nobody else uses this branch
                child.reset();        // frees the whole dead subtree
                --words_;
                return true;
            }
            cur = nxt;
        }
        cur->end = false;             // word was a prefix of other words
        --words_;
        return true;
    }

    // Up to `limit` words starting with prefix, in alphabetical order.
    std::vector<std::string> autocomplete(const std::string& prefix,
                                          size_t limit = 10) const {
        std::vector<std::string> out;
        const Node* n = find(prefix);
        if (!n) return out;
        std::string cur = prefix;
        collect(n, cur, out, limit);
        return out;
    }

    // Longest common prefix shared by ALL stored words.
    std::string longest_common_prefix() const {
        std::string res;
        const Node* cur = root_.get();
        while (cur && !cur->end) {
            const Node* only = nullptr;
            int children = 0;
            char which = 0;
            for (int c = 0; c < 26; ++c)
                if (cur->next[c]) {
                    ++children;
                    only = cur->next[c].get();
                    which = static_cast<char>('a' + c);
                }
            if (children != 1) break;
            res.push_back(which);
            cur = only;
        }
        return res;
    }
};

// ===========================================================================
//  Part 2 + 3: Suffix array, LCP array
// ===========================================================================
class SuffixArray {
public:
    explicit SuffixArray(std::string s) : s_(std::move(s)) {
        build();
        buildLcp();
        buildSparse();
    }

    int size() const { return n_; }
    const std::vector<int>& sa() const { return sa_; }
    const std::vector<int>& rank() const { return rank_; }
    // lcp()[i] = LCP(suffix sa[i], suffix sa[i+1]); length n-1 (or 0).
    const std::vector<int>& lcp() const { return lcp_; }

    std::string suffix(int i) const { return s_.substr(sa_[i]); }

    // LCP of the suffixes starting at text positions a and b, O(1).
    int lcp_of_suffixes(int a, int b) const {
        if (a == b) return n_ - a;
        int ra = rank_[a], rb = rank_[b];
        if (ra > rb) std::swap(ra, rb);
        // minimum of lcp_[ra .. rb-1]
        int len = rb - ra;
        int k = log_[len];
        return std::min(sparse_[k][ra], sparse_[k][rb - (1 << k)]);
    }

    // Half-open range [lo, hi) of suffix-array positions whose suffix begins
    // with pattern. Two binary searches, O(|p| log n).
    std::pair<int, int> equal_range(const std::string& p) const {
        auto lowerCmp = [&](int pos) {  // suffix < pattern (prefix-wise)?
            return s_.compare(pos, p.size(), p) < 0;
        };
        auto upperCmp = [&](int pos) {  // suffix prefix <= pattern?
            return s_.compare(pos, p.size(), p) <= 0;
        };
        int lo = 0, hi = n_;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (lowerCmp(sa_[mid])) lo = mid + 1; else hi = mid;
        }
        int first = lo;
        hi = n_;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (upperCmp(sa_[mid])) lo = mid + 1; else hi = mid;
        }
        return {first, lo};
    }

    int count_occurrences(const std::string& p) const {
        auto r = equal_range(p);
        return r.second - r.first;
    }

    // Sorted starting positions of every occurrence of p.
    std::vector<int> occurrences(const std::string& p) const {
        auto r = equal_range(p);
        std::vector<int> pos(sa_.begin() + r.first, sa_.begin() + r.second);
        std::sort(pos.begin(), pos.end());
        return pos;
    }

    // Number of distinct non-empty substrings.
    long long distinct_substrings() const {
        long long total = static_cast<long long>(n_) * (n_ + 1) / 2;
        for (int v : lcp_) total -= v;
        return total;
    }

    // Longest substring that occurs at least twice (may overlap).
    std::string longest_repeated_substring() const {
        int best = 0, at = 0;
        for (int i = 0; i + 1 < n_; ++i)
            if (lcp_[i] > best) {
                best = lcp_[i];
                at = sa_[i];
            }
        return s_.substr(at, best);
    }

private:
    std::string s_;
    int n_ = 0;
    std::vector<int> sa_, rank_, lcp_;
    std::vector<std::vector<int>> sparse_;
    std::vector<int> log_;

    // Prefix doubling: sort suffixes by their first 2^k characters, using
    // counting sort on (rank[i], rank[i+2^k]) pairs.
    void build() {
        n_ = static_cast<int>(s_.size());
        sa_.assign(n_, 0);
        rank_.assign(n_, 0);
        if (n_ == 0) return;

        const int ALPHA = 256;
        int classes = std::max(ALPHA, n_) + 1;
        std::vector<int> cnt(classes, 0), tmp(n_), sa2(n_);

        // Initial ordering by single character.
        for (int i = 0; i < n_; ++i) ++cnt[static_cast<unsigned char>(s_[i])];
        for (int i = 1; i < ALPHA; ++i) cnt[i] += cnt[i - 1];
        for (int i = n_ - 1; i >= 0; --i)
            sa_[--cnt[static_cast<unsigned char>(s_[i])]] = i;
        rank_[sa_[0]] = 0;
        int numClasses = 1;
        for (int i = 1; i < n_; ++i) {
            if (s_[sa_[i]] != s_[sa_[i - 1]]) ++numClasses;
            rank_[sa_[i]] = numClasses - 1;
        }

        for (int len = 1; len < n_ && numClasses < n_; len <<= 1) {
            // Sort by second key: the suffix i's second half starts at i+len.
            // Suffixes with no second half (i+len >= n) sort first.
            int p = 0;
            for (int i = n_ - len; i < n_; ++i) sa2[p++] = i;
            for (int i = 0; i < n_; ++i)
                if (sa_[i] >= len) sa2[p++] = sa_[i] - len;

            // Stable counting sort by first key (current rank).
            std::fill(cnt.begin(), cnt.begin() + numClasses, 0);
            for (int i = 0; i < n_; ++i) ++cnt[rank_[i]];
            for (int i = 1; i < numClasses; ++i) cnt[i] += cnt[i - 1];
            for (int i = n_ - 1; i >= 0; --i) sa_[--cnt[rank_[sa2[i]]]] = sa2[i];

            // Recompute equivalence classes.
            tmp[sa_[0]] = 0;
            int c = 1;
            for (int i = 1; i < n_; ++i) {
                int a1 = rank_[sa_[i]], b1 = rank_[sa_[i - 1]];
                int a2 = sa_[i] + len < n_ ? rank_[sa_[i] + len] : -1;
                int b2 = sa_[i - 1] + len < n_ ? rank_[sa_[i - 1] + len] : -1;
                if (a1 != b1 || a2 != b2) ++c;
                tmp[sa_[i]] = c - 1;
            }
            rank_ = tmp;
            numClasses = c;
        }
        for (int i = 0; i < n_; ++i) rank_[sa_[i]] = i;  // final: exact rank
    }

    // Kasai's algorithm: process suffixes in text order; the LCP can drop by at
    // most one when moving from suffix i to suffix i+1, giving O(n) total.
    void buildLcp() {
        lcp_.assign(n_ > 0 ? n_ - 1 : 0, 0);
        int h = 0;
        for (int i = 0; i < n_; ++i) {
            if (rank_[i] + 1 < n_) {
                int j = sa_[rank_[i] + 1];  // next suffix in sorted order
                while (i + h < n_ && j + h < n_ && s_[i + h] == s_[j + h]) ++h;
                lcp_[rank_[i]] = h;
                if (h > 0) --h;
            } else {
                h = 0;
            }
        }
    }

    // Sparse table over lcp_ for O(1) range-minimum queries.
    void buildSparse() {
        int m = static_cast<int>(lcp_.size());
        log_.assign(m + 2, 0);
        for (int i = 2; i <= m + 1; ++i) log_[i] = log_[i / 2] + 1;
        sparse_.clear();
        if (m == 0) return;
        sparse_.push_back(lcp_);
        for (int k = 1; (1 << k) <= m; ++k) {
            std::vector<int> row(m - (1 << k) + 1);
            for (int i = 0; i + (1 << k) <= m; ++i)
                row[i] = std::min(sparse_[k - 1][i],
                                  sparse_[k - 1][i + (1 << (k - 1))]);
            sparse_.push_back(std::move(row));
        }
    }
};

// Longest common substring of a and b using a suffix array on a + '#' + b.
static std::string longestCommonSubstring(const std::string& a,
                                          const std::string& b) {
    std::string t = a + '#' + b;  // '#' occurs in neither input
    SuffixArray sa(t);
    int na = static_cast<int>(a.size());
    int best = 0, at = 0;
    for (int i = 0; i + 1 < sa.size(); ++i) {
        int p = sa.sa()[i], q = sa.sa()[i + 1];
        bool differentSides = (p < na) != (q < na);  // one in a, one in b
        if (differentSides && sa.lcp()[i] > best) {
            best = sa.lcp()[i];
            at = std::min(p, q) < na ? (p < na ? p : q) : p;
        }
    }
    return t.substr(at, best);
}

// ===========================================================================
//  Test utilities
// ===========================================================================
static int g_failures = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            ++g_failures;                                                 \
            std::cout << "  [FAIL] line " << __LINE__ << ": " #cond "\n"; \
        }                                                                 \
    } while (0)

template <typename T>
static std::string show(const std::vector<T>& v) {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) os << (i ? ", " : "") << v[i];
    os << "]";
    return os.str();
}

// Brute-force helpers for verification.
static std::vector<int> naiveSA(const std::string& s) {
    std::vector<int> sa(s.size());
    std::iota(sa.begin(), sa.end(), 0);
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return s.compare(a, std::string::npos, s, b, std::string::npos) < 0;
    });
    return sa;
}

static int naiveLcp(const std::string& s, int a, int b) {
    int h = 0;
    while (a + h < (int)s.size() && b + h < (int)s.size() && s[a + h] == s[b + h]) ++h;
    return h;
}

// ===========================================================================
//  Tests
// ===========================================================================
static void testTrie() {
    std::cout << "== Trie ==\n";
    Trie t;
    for (const char* w : {"apple", "app", "application", "apt", "banana",
                          "band", "bandana", "can"})
        CHECK(t.insert(w));
    CHECK(!t.insert("app"));   // duplicate
    CHECK(!t.insert("Bad"));   // invalid character
    CHECK(t.size() == 8);
    CHECK(t.contains("app"));
    CHECK(!t.contains("appl"));
    CHECK(t.count_with_prefix("app") == 3);
    CHECK(t.count_with_prefix("ban") == 3);
    CHECK(t.count_with_prefix("z") == 0);
    CHECK(t.count_with_prefix("") == 8);

    auto ac = t.autocomplete("app");
    std::cout << "  autocomplete(app): " << show(ac) << "\n";
    CHECK((ac == std::vector<std::string>{"app", "apple", "application"}));
    CHECK(t.autocomplete("ban", 2).size() == 2);
    CHECK(t.autocomplete("x").empty());

    CHECK(t.erase("app"));       // prefix of other words: node stays
    CHECK(!t.contains("app"));
    CHECK(t.contains("apple"));
    CHECK(t.count_with_prefix("app") == 2);
    CHECK(t.erase("can"));       // whole branch pruned
    CHECK(!t.erase("can"));
    CHECK(t.count_with_prefix("c") == 0);
    CHECK(t.size() == 6);

    Trie lcp;
    for (const char* w : {"flower", "flow", "flight"}) lcp.insert(w);
    std::cout << "  LCP of words     : '" << lcp.longest_common_prefix() << "'\n";
    CHECK(lcp.longest_common_prefix() == "fl");
}

static void testSuffixArrayBasics() {
    std::cout << "== Suffix array / LCP basics (banana) ==\n";
    SuffixArray sa("banana");
    std::cout << "  sa  : " << show(sa.sa()) << "\n";
    std::cout << "  lcp : " << show(sa.lcp()) << "\n";
    for (int i = 0; i < sa.size(); ++i)
        std::cout << "    " << i << ": " << sa.suffix(i) << "\n";
    CHECK((sa.sa() == std::vector<int>{5, 3, 1, 0, 4, 2}));
    CHECK((sa.lcp() == std::vector<int>{1, 3, 0, 0, 2}));
    CHECK(sa.distinct_substrings() == 15);
    CHECK(sa.longest_repeated_substring() == "ana");
    CHECK(sa.count_occurrences("ana") == 2);
    CHECK(sa.count_occurrences("a") == 3);
    CHECK(sa.count_occurrences("nan") == 1);
    CHECK(sa.count_occurrences("xyz") == 0);
    CHECK(sa.count_occurrences("bananas") == 0);
    CHECK((sa.occurrences("ana") == std::vector<int>{1, 3}));
    CHECK(sa.lcp_of_suffixes(1, 3) == 3);  // "anana" vs "ana"
    CHECK(sa.lcp_of_suffixes(0, 2) == 0);
    CHECK(sa.lcp_of_suffixes(4, 4) == 2);
    std::cout << "  distinct substrings: " << sa.distinct_substrings() << "\n";
    std::cout << "  longest repeated   : " << sa.longest_repeated_substring() << "\n";
}

static void testSuffixArrayEdge() {
    std::cout << "== Suffix array edge cases ==\n";
    SuffixArray empty("");
    CHECK(empty.size() == 0);
    CHECK(empty.distinct_substrings() == 0);
    CHECK(empty.count_occurrences("a") == 0);

    SuffixArray one("x");
    CHECK((one.sa() == std::vector<int>{0}));
    CHECK(one.distinct_substrings() == 1);

    SuffixArray same(std::string(10, 'a'));  // worst case for doubling
    CHECK(same.distinct_substrings() == 10);
    CHECK((same.sa() == std::vector<int>{9, 8, 7, 6, 5, 4, 3, 2, 1, 0}));
    CHECK(same.count_occurrences("aaa") == 8);
    std::cout << "  aaaaaaaaaa sa: " << show(same.sa()) << "\n";
}

static void testLongestCommonSubstring() {
    std::cout << "== Longest common substring ==\n";
    std::string r = longestCommonSubstring("xabcdefy", "zzcdefgabq");
    std::cout << "  lcs(xabcdefy, zzcdefgabq) = " << r << "\n";
    CHECK(r == "cdef");
    CHECK(longestCommonSubstring("abc", "xyz").empty());
    CHECK(longestCommonSubstring("hello", "hello") == "hello");
}

static void testRandomSuffixArrays() {
    std::cout << "== Randomized suffix array vs naive ==\n";
    std::mt19937 rng(2024);
    int bad = 0, cases = 0;
    for (int iter = 0; iter < 400; ++iter) {
        int n = 1 + static_cast<int>(rng() % 60);
        int alpha = 1 + static_cast<int>(rng() % 4);  // small alphabets: many repeats
        std::string s(n, 'a');
        for (auto& c : s) c = static_cast<char>('a' + rng() % alpha);
        SuffixArray sa(s);
        ++cases;
        if (sa.sa() != naiveSA(s)) { ++bad; continue; }
        // Neighbour LCPs.
        for (int i = 0; i + 1 < n; ++i)
            if (sa.lcp()[i] != naiveLcp(s, sa.sa()[i], sa.sa()[i + 1])) ++bad;
        // Arbitrary suffix pair LCP via sparse table.
        for (int q = 0; q < 20; ++q) {
            int a = static_cast<int>(rng() % n), b = static_cast<int>(rng() % n);
            if (sa.lcp_of_suffixes(a, b) != naiveLcp(s, a, b)) ++bad;
        }
        // Distinct substrings.
        std::set<std::string> subs;
        for (int i = 0; i < n; ++i)
            for (int len = 1; i + len <= n; ++len) subs.insert(s.substr(i, len));
        if ((long long)subs.size() != sa.distinct_substrings()) ++bad;
        // Pattern counts.
        std::string p = s.substr(rng() % n, 1 + rng() % 3);
        int expected = 0;
        for (size_t pos = s.find(p); pos != std::string::npos; pos = s.find(p, pos + 1))
            ++expected;
        if (sa.count_occurrences(p) != expected) ++bad;
    }
    CHECK(bad == 0);
    std::cout << "  cases=" << cases << " problems=" << bad << "\n";
}

static void testLarge() {
    std::cout << "== Large input ==\n";
    std::mt19937 rng(5);
    std::string s(200000, 'a');
    for (auto& c : s) c = static_cast<char>('a' + rng() % 3);
    SuffixArray sa(s);
    bool sorted = true;
    for (int i = 0; i + 1 < sa.size() && i < 2000; ++i)
        if (s.compare(sa.sa()[i], std::string::npos, s, sa.sa()[i + 1],
                      std::string::npos) >= 0)
            sorted = false;
    CHECK(sorted);
    std::cout << "  n=" << sa.size()
              << " longest repeat length=" << sa.longest_repeated_substring().size()
              << "\n";
}

int main() {
    std::cout << "Trie, suffix array and LCP array\n\n";
    testTrie();
    testSuffixArrayBasics();
    testSuffixArrayEdge();
    testLongestCommonSubstring();
    testRandomSuffixArrays();
    testLarge();
    std::cout << "\n" << (g_failures == 0 ? "ALL TESTS PASSED"
                                          : "SOME TESTS FAILED")
              << " (failures: " << g_failures << ")\n";
    return g_failures == 0 ? 0 : 1;
}
