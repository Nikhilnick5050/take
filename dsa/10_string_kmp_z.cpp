/*
 * 10_string_kmp_z.cpp
 *
 * Advanced string matching and string structure algorithms:
 *   1. Knuth-Morris-Pratt (KMP) prefix function and matching
 *   2. Z-function and pattern matching with Z
 *   3. Rabin-Karp rolling hash with double modulus
 *   4. Aho-Corasick automaton for many patterns at once
 *   5. Minimal rotation (Booth's algorithm)
 *   6. Count of distinct substrings using the Z-function
 *
 * All matchers are checked against a naive brute force search.
 *
 * Compile:  g++ -std=c++17 -Wall -Wextra -O2 10_string_kmp_z.cpp -o strmatch
 * Run:      ./strmatch
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

using namespace std;

using Matches = vector<int>;  // starting indices of matches in the text

// ---------------------------------------------------------------------------
// Naive reference: O(n * m). Used only for testing.
// ---------------------------------------------------------------------------
Matches naiveSearch(const string& text, const string& pattern) {
    Matches out;
    if (pattern.empty()) return out;
    for (size_t i = 0; i + pattern.size() <= text.size(); ++i) {
        if (text.compare(i, pattern.size(), pattern) == 0) {
            out.push_back(static_cast<int>(i));
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// 1. KMP
// ---------------------------------------------------------------------------

// pi[i] = length of the longest proper prefix of s[0..i] that is also a suffix.
vector<int> prefixFunction(const string& s) {
    int n = static_cast<int>(s.size());
    vector<int> pi(n, 0);
    for (int i = 1; i < n; ++i) {
        int k = pi[i - 1];
        while (k > 0 && s[i] != s[k]) {
            k = pi[k - 1];
        }
        if (s[i] == s[k]) ++k;
        pi[i] = k;
    }
    return pi;
}

// Find every occurrence of pattern in text in O(n + m).
Matches kmpSearch(const string& text, const string& pattern) {
    Matches out;
    if (pattern.empty() || pattern.size() > text.size()) return out;

    // Build the string pattern + '#' + text is one approach, but a direct
    // scan over pattern's prefix table avoids a separator character.
    vector<int> pi = prefixFunction(pattern);
    int m = static_cast<int>(pattern.size());
    int k = 0;
    for (int i = 0; i < static_cast<int>(text.size()); ++i) {
        while (k > 0 && text[i] != pattern[k]) {
            k = pi[k - 1];
        }
        if (text[i] == pattern[k]) ++k;
        if (k == m) {
            out.push_back(i - m + 1);
            k = pi[k - 1];
        }
    }
    return out;
}

// Smallest period of s (length of the smallest repeating unit that tiles s,
// even if the last repeat is partial). Derived from the prefix function.
int smallestPeriod(const string& s) {
    if (s.empty()) return 0;
    vector<int> pi = prefixFunction(s);
    int n = static_cast<int>(s.size());
    return n - pi[n - 1];
}

// Number of distinct non-empty substrings of s, by collecting every substring
// in a set. Simple and exact; used as the reference answer in tests.
long long countDistinctSubstringsBrute(const string& s) {
    set<string> all;
    for (size_t i = 0; i < s.size(); ++i) {
        for (size_t len = 1; i + len <= s.size(); ++len) {
            all.insert(s.substr(i, len));
        }
    }
    return static_cast<long long>(all.size());
}

// ---------------------------------------------------------------------------
// 2. Z-function
// ---------------------------------------------------------------------------

// z[i] = length of the longest common prefix of s and s[i..].
// z[0] is defined as n.
vector<int> zFunction(const string& s) {
    int n = static_cast<int>(s.size());
    vector<int> z(n, 0);
    if (n == 0) return z;
    z[0] = n;
    int l = 0, r = 0;  // [l, r) is the rightmost Z-box found so far
    for (int i = 1; i < n; ++i) {
        if (i < r) {
            z[i] = min(r - i, z[i - l]);
        }
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        if (i + z[i] > r) {
            l = i;
            r = i + z[i];
        }
    }
    return z;
}

// Pattern matching with Z: build pattern + sep + text and look for values
// equal to the pattern length. The separator must not occur in either string.
Matches zSearch(const string& text, const string& pattern) {
    Matches out;
    if (pattern.empty() || pattern.size() > text.size()) return out;
    const char sep = '\x01';
    string combined = pattern + sep + text;
    vector<int> z = zFunction(combined);
    int m = static_cast<int>(pattern.size());
    for (int i = m + 1; i < static_cast<int>(combined.size()); ++i) {
        if (z[i] >= m) {
            out.push_back(i - m - 1);
        }
    }
    return out;
}

// Count distinct substrings using the Z-function: for each suffix starting at
// i, add (n - i) - max(z[j]) over the already processed suffixes. Here we use
// the simpler formula: distinct = sum over i of (n - i - maxPrefixWithEarlier).
// This version is O(n^2) in the worst case and is kept as a clear reference.
long long countDistinctSubstringsZ(const string& s) {
    int n = static_cast<int>(s.size());
    long long total = 0;
    for (int i = 0; i < n; ++i) {
        string suffix = s.substr(i);
        vector<int> z = zFunction(suffix);
        int best = 0;
        // For a new suffix, its contribution is its length minus the longest
        // prefix it shares with any earlier suffix (checked by prefix matching).
        for (int j = 0; j < i; ++j) {
            string earlier = s.substr(j);
            int k = 0;
            while (k < static_cast<int>(suffix.size()) &&
                   k < static_cast<int>(earlier.size()) &&
                   suffix[k] == earlier[k]) {
                ++k;
            }
            best = max(best, k);
        }
        total += static_cast<long long>(n - i) - best;
    }
    return total;
}

// ---------------------------------------------------------------------------
// 3. Rabin-Karp with double modular hash
// ---------------------------------------------------------------------------
class RabinKarp {
public:
    static constexpr uint64_t M1 = 1000000007ULL;
    static constexpr uint64_t M2 = 998244353ULL;
    static constexpr uint64_t B1 = 131ULL;
    static constexpr uint64_t B2 = 137ULL;

    // Find all occurrences. Two independent hashes make collisions negligible.
    static Matches search(const string& text, const string& pattern) {
        Matches out;
        int n = static_cast<int>(text.size());
        int m = static_cast<int>(pattern.size());
        if (m == 0 || m > n) return out;

        uint64_t ph1 = 0, ph2 = 0, th1 = 0, th2 = 0, pow1 = 1, pow2 = 1;
        for (int i = 0; i < m; ++i) {
            ph1 = (ph1 * B1 + static_cast<unsigned char>(pattern[i])) % M1;
            ph2 = (ph2 * B2 + static_cast<unsigned char>(pattern[i])) % M2;
            th1 = (th1 * B1 + static_cast<unsigned char>(text[i])) % M1;
            th2 = (th2 * B2 + static_cast<unsigned char>(text[i])) % M2;
            if (i + 1 < m) {
                pow1 = pow1 * B1 % M1;
                pow2 = pow2 * B2 % M2;
            }
        }
        for (int i = 0;; ++i) {
            if (th1 == ph1 && th2 == ph2) {
                if (text.compare(i, m, pattern) == 0) {  // verify to be exact
                    out.push_back(i);
                }
            }
            if (i + m >= n) break;
            // Roll the window: remove text[i], add text[i + m].
            uint64_t out1 = static_cast<unsigned char>(text[i]) * pow1 % M1;
            uint64_t out2 = static_cast<unsigned char>(text[i]) * pow2 % M2;
            th1 = (th1 + M1 - out1) % M1;
            th2 = (th2 + M2 - out2) % M2;
            th1 = (th1 * B1 + static_cast<unsigned char>(text[i + m])) % M1;
            th2 = (th2 * B2 + static_cast<unsigned char>(text[i + m])) % M2;
        }
        return out;
    }

    // Longest common substring length between a and b using binary search
    // on the length and a hash set of substrings of a.
    static int longestCommonSubstring(const string& a, const string& b) {
        int lo = 0, hi = static_cast<int>(min(a.size(), b.size()));
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            if (hasCommonOfLength(a, b, mid)) {
                lo = mid;
            } else {
                hi = mid - 1;
            }
        }
        return lo;
    }

private:
    static bool hasCommonOfLength(const string& a, const string& b, int len) {
        if (len == 0) return true;
        set<pair<uint64_t, uint64_t>> seen;
        int na = static_cast<int>(a.size());
        int nb = static_cast<int>(b.size());
        if (len > na || len > nb) return false;

        uint64_t pow1 = 1, pow2 = 1;
        for (int i = 0; i < len - 1; ++i) {
            pow1 = pow1 * B1 % M1;
            pow2 = pow2 * B2 % M2;
        }
        auto hashWindow = [&](const string& s, int start, uint64_t& h1, uint64_t& h2) {
            h1 = 0;
            h2 = 0;
            for (int i = 0; i < len; ++i) {
                h1 = (h1 * B1 + static_cast<unsigned char>(s[start + i])) % M1;
                h2 = (h2 * B2 + static_cast<unsigned char>(s[start + i])) % M2;
            }
        };
        uint64_t h1 = 0, h2 = 0;
        hashWindow(a, 0, h1, h2);
        seen.insert({h1, h2});
        for (int i = 1; i + len <= na; ++i) {
            uint64_t o1 = static_cast<unsigned char>(a[i - 1]) * pow1 % M1;
            uint64_t o2 = static_cast<unsigned char>(a[i - 1]) * pow2 % M2;
            h1 = (h1 + M1 - o1) % M1;
            h2 = (h2 + M2 - o2) % M2;
            h1 = (h1 * B1 + static_cast<unsigned char>(a[i + len - 1])) % M1;
            h2 = (h2 * B2 + static_cast<unsigned char>(a[i + len - 1])) % M2;
            seen.insert({h1, h2});
        }
        hashWindow(b, 0, h1, h2);
        if (seen.count({h1, h2})) return true;
        for (int i = 1; i + len <= nb; ++i) {
            uint64_t o1 = static_cast<unsigned char>(b[i - 1]) * pow1 % M1;
            uint64_t o2 = static_cast<unsigned char>(b[i - 1]) * pow2 % M2;
            h1 = (h1 + M1 - o1) % M1;
            h2 = (h2 + M2 - o2) % M2;
            h1 = (h1 * B1 + static_cast<unsigned char>(b[i + len - 1])) % M1;
            h2 = (h2 * B2 + static_cast<unsigned char>(b[i + len - 1])) % M2;
            if (seen.count({h1, h2})) return true;
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
// 4. Aho-Corasick automaton
// ---------------------------------------------------------------------------
class AhoCorasick {
public:
    AhoCorasick() { newNode(); }

    // Add a pattern. Returns the pattern index.
    int addPattern(const string& p) {
        int node = 0;
        for (char ch : p) {
            int c = static_cast<unsigned char>(ch);
            if (nodes_[node].next[c] == -1) {
                int created = newNode();
                nodes_[node].next[c] = created;
            }
            node = nodes_[node].next[c];
        }
        nodes_[node].terminal.push_back(static_cast<int>(patterns_.size()));
        patterns_.push_back(p);
        built_ = false;
        return static_cast<int>(patterns_.size()) - 1;
    }

    // Build failure links with BFS. Must be called after all patterns are added.
    void build() {
        queue<int> q;
        for (int c = 0; c < ALPHA; ++c) {
            int v = nodes_[0].next[c];
            if (v == -1) {
                nodes_[0].next[c] = 0;
            } else {
                nodes_[v].fail = 0;
                q.push(v);
            }
        }
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            // Output of a node includes outputs reachable through failure links.
            int f = nodes_[u].fail;
            nodes_[u].dictLink = nodes_[f].terminal.empty() ? nodes_[f].dictLink : f;
            for (int c = 0; c < ALPHA; ++c) {
                int v = nodes_[u].next[c];
                if (v == -1) {
                    nodes_[u].next[c] = nodes_[f].next[c];
                } else {
                    nodes_[v].fail = nodes_[f].next[c];
                    q.push(v);
                }
            }
        }
        built_ = true;
    }

    // Returns pairs (end position, pattern index) for every match in text.
    vector<pair<int, int>> search(const string& text) const {
        assert(built_);
        vector<pair<int, int>> out;
        int node = 0;
        for (int i = 0; i < static_cast<int>(text.size()); ++i) {
            node = nodes_[node].next[static_cast<unsigned char>(text[i])];
            // Report terminal patterns at this node, then along dictionary links.
            for (int p : nodes_[node].terminal) out.push_back({i, p});
            int d = nodes_[node].dictLink;
            while (d > 0) {
                for (int p : nodes_[d].terminal) out.push_back({i, p});
                d = nodes_[d].dictLink;
            }
        }
        return out;
    }

    const string& pattern(int idx) const { return patterns_[idx]; }
    size_t nodeCount() const { return nodes_.size(); }

private:
    static constexpr int ALPHA = 256;

    struct Node {
        array<int, ALPHA> next;
        int fail = 0;
        int dictLink = 0;
        vector<int> terminal;
        Node() { next.fill(-1); }
    };

    int newNode() {
        nodes_.emplace_back();
        return static_cast<int>(nodes_.size()) - 1;
    }

    vector<Node> nodes_;
    vector<string> patterns_;
    bool built_ = false;
};

// Reference for Aho-Corasick: naive search per pattern.
vector<pair<int, int>> naiveMulti(const string& text, const vector<string>& pats) {
    vector<pair<int, int>> out;
    for (size_t p = 0; p < pats.size(); ++p) {
        if (pats[p].empty()) continue;
        for (int start : naiveSearch(text, pats[p])) {
            out.push_back({start + static_cast<int>(pats[p].size()) - 1, static_cast<int>(p)});
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// 5. Minimal rotation (Booth's algorithm, O(n))
// ---------------------------------------------------------------------------
int minimalRotationIndex(const string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return 0;
    string d = s + s;
    int i = 0, j = 1, k = 0;
    while (i < n && j < n && k < n) {
        char a = d[i + k];
        char b = d[j + k];
        if (a == b) {
            ++k;
            continue;
        }
        if (a > b) {
            i += k + 1;
        } else {
            j += k + 1;
        }
        if (i == j) ++j;
        k = 0;
    }
    return min(i, j);
}

string minimalRotation(const string& s) {
    if (s.empty()) return s;
    int idx = minimalRotationIndex(s);
    return s.substr(idx) + s.substr(0, idx);
}

// ---------------------------------------------------------------------------
// Test helpers
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

string randomString(unsigned& seed, int len, int alphabet) {
    string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i) {
        seed = seed * 1103515245u + 12345u;
        s.push_back(static_cast<char>('a' + (seed >> 16) % alphabet));
    }
    return s;
}

void printMatches(const string& label, const Matches& m) {
    cout << "  " << label << ": [";
    for (size_t i = 0; i < m.size(); ++i) {
        cout << m[i] << (i + 1 < m.size() ? ", " : "");
    }
    cout << "]\n";
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------
void testPrefixFunction() {
    cout << "\n== Prefix function (KMP table) ==\n";
    vector<int> pi = prefixFunction("aabaaab");
    vector<int> expected = {0, 1, 0, 1, 2, 2, 3};
    check(pi == expected, "prefix function of aabaaab");
    check(smallestPeriod("abcabcab") == 3, "smallest period of abcabcab is 3");
    check(smallestPeriod("aaaa") == 1, "smallest period of aaaa is 1");
    check(smallestPeriod("abcd") == 4, "smallest period of abcd is 4");
}

void testKmpExample() {
    cout << "\n== KMP search: example ==\n";
    string text = "abracadabra abracadabra";
    Matches got = kmpSearch(text, "abra");
    printMatches("matches of abra", got);
    check(got == Matches({0, 7, 12, 19}), "KMP finds all four abra occurrences");
}

void testZFunction() {
    cout << "\n== Z-function ==\n";
    vector<int> z = zFunction("aaabaab");
    vector<int> expected = {7, 2, 1, 0, 2, 1, 0};
    check(z == expected, "Z-array of aaabaab");
    Matches got = zSearch("aaabaaab", "aab");
    printMatches("Z search for aab in aaabaaab", got);
    check(got == Matches({1, 5}), "Z search finds aab at 1 and 5");
}

void testRandomMatchers() {
    cout << "\n== Random tests: KMP, Z, Rabin-Karp vs naive ==\n";
    unsigned seed = 2024;
    bool kmpOk = true, zOk = true, rkOk = true;
    for (int trial = 0; trial < 300; ++trial) {
        int n = 1 + static_cast<int>(seed % 60);
        seed = seed * 1103515245u + 12345u;
        string text = randomString(seed, n, 2);
        int m = 1 + static_cast<int>(seed % 5);
        seed = seed * 1103515245u + 12345u;
        string pat = randomString(seed, m, 2);
        Matches expect = naiveSearch(text, pat);
        if (kmpSearch(text, pat) != expect) kmpOk = false;
        if (zSearch(text, pat) != expect) zOk = false;
        if (RabinKarp::search(text, pat) != expect) rkOk = false;
    }
    check(kmpOk, "KMP matches naive on 300 random cases");
    check(zOk, "Z search matches naive on 300 random cases");
    check(rkOk, "Rabin-Karp matches naive on 300 random cases");
}

void testRabinKarpLcs() {
    cout << "\n== Rabin-Karp: longest common substring ==\n";
    int len = RabinKarp::longestCommonSubstring("xabcdyz", "pqabcdr");
    cout << "  length: " << len << "\n";
    check(len == 4, "longest common substring of xabcdyz and pqabcdr is abcd");
    check(RabinKarp::longestCommonSubstring("abc", "xyz") == 0, "no common letters gives 0");
}

void testAhoCorasick() {
    cout << "\n== Aho-Corasick: multi-pattern search ==\n";
    vector<string> pats = {"he", "she", "his", "hers"};
    AhoCorasick ac;
    for (const string& p : pats) ac.addPattern(p);
    ac.build();
    string text = "ushers";
    auto got = ac.search(text);
    auto expect = naiveMulti(text, pats);
    cout << "  matches (end index, pattern):";
    for (auto& [e, p] : got) cout << " (" << e << "," << ac.pattern(p) << ")";
    cout << "\n";
    sort(got.begin(), got.end());
    sort(expect.begin(), expect.end());
    check(got == expect, "Aho-Corasick finds the same matches as naive search");
    check(got.size() == 3, "ushers contains she and he (end 3) and hers (end 5)");
}

void testAhoCorasickRandom() {
    cout << "\n== Aho-Corasick: random tests vs naive ==\n";
    unsigned seed = 77;
    bool ok = true;
    for (int trial = 0; trial < 100; ++trial) {
        vector<string> pats;
        int k = 1 + static_cast<int>(seed % 6);
        for (int i = 0; i < k; ++i) {
            seed = seed * 1103515245u + 12345u;
            int len = 1 + static_cast<int>((seed >> 8) % 4);
            pats.push_back(randomString(seed, len, 3));
        }
        seed = seed * 1103515245u + 12345u;
        string text = randomString(seed, 40, 3);

        AhoCorasick ac;
        for (const string& p : pats) ac.addPattern(p);
        ac.build();
        auto got = ac.search(text);
        auto expect = naiveMulti(text, pats);
        sort(got.begin(), got.end());
        sort(expect.begin(), expect.end());
        if (got != expect) ok = false;
    }
    check(ok, "Aho-Corasick matches naive on 100 random pattern sets");
}

void testMinimalRotation() {
    cout << "\n== Minimal rotation (Booth) ==\n";
    string r = minimalRotation("bcabca");
    cout << "  minimal rotation of bcabca: " << r << "\n";
    check(r == "abcabc", "minimal rotation of bcabca is abcabc");
    check(minimalRotation("dcba") == "adcb", "minimal rotation of dcba is adcb");
    check(minimalRotation("") == "", "empty string stays empty");
}

void testDistinctSubstrings() {
    cout << "\n== Distinct substrings ==\n";
    check(countDistinctSubstringsBrute("ababa") == 9, "ababa has 9 distinct substrings");
    check(countDistinctSubstringsZ("ababa") == 9, "Z method also gives 9 for ababa");
    check(countDistinctSubstringsBrute("aaaa") == 4, "aaaa has 4 distinct substrings");
    bool agree = true;
    unsigned seed = 5;
    for (int t = 0; t < 30; ++t) {
        string s = randomString(seed, 1 + static_cast<int>(seed % 12), 2);
        if (countDistinctSubstringsBrute(s) != countDistinctSubstringsZ(s)) agree = false;
    }
    check(agree, "Z-based distinct count matches the set-based reference on 30 random strings");
}

int main() {
    cout << "10 - String algorithms: KMP, Z, Rabin-Karp, Aho-Corasick, Booth\n";

    testPrefixFunction();
    testKmpExample();
    testZFunction();
    testRandomMatchers();
    testRabinKarpLcs();
    testAhoCorasick();
    testAhoCorasickRandom();
    testMinimalRotation();
    testDistinctSubstrings();

    cout << "\nSummary: " << testsPassed << " / " << testsRun << " checks passed\n";
    return testsPassed == testsRun ? 0 : 1;
}
