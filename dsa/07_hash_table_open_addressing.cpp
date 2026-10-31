/*
 * ============================================================================
 *  File    : 07_hash_table_open_addressing.cpp
 *  Topic   : Hash tables with open addressing
 *  Standard: C++17
 *  Build   : g++ -std=c++17 -Wall -Wextra 07_hash_table_open_addressing.cpp -o ht
 * ----------------------------------------------------------------------------
 *  Contents
 *    1. Probe strategies: linear probing and double hashing
 *    2. A generic OpenHashTable<K, V, Probe> template with
 *         - three slot states: EMPTY, OCCUPIED, DELETED (tombstone)
 *         - insert / find / erase / operator[] / contains
 *         - automatic rehash when the load factor (including tombstones)
 *           exceeds a threshold; capacity growth to the next prime or
 *           power-of-two depending on the probe strategy
 *         - statistics: probe counts, longest probe, load factor
 *    3. Experiments: primary clustering, tombstone behaviour, rehash,
 *       comparison against std::unordered_map under random operations
 *
 *  Probing sequences (h1 = primary hash, h2 = secondary hash, m = capacity):
 *      linear        : (h1 + i) mod m
 *      double hashing: (h1 + i * h2) mod m, with h2 in [1, m-1] and
 *                      gcd(h2, m) = 1 so that the sequence visits all slots.
 *                      Using a prime m guarantees this for any h2 in [1,m-1].
 *
 *  For simplicity, the table capacity is always prime for both strategies.
 * ============================================================================
 */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

// ============================================================================
// Section 1: helpers - primes and hash mixing
// ============================================================================

static bool isPrime(size_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (size_t d = 3; d * d <= n; d += 2)
        if (n % d == 0) return false;
    return true;
}

static size_t nextPrime(size_t n) {
    while (!isPrime(n)) ++n;
    return n;
}

// SplitMix64 finaliser: spreads the bits of std::hash (which is the identity
// for integers on common standard libraries) so keys such as 0,1,2,... do not
// map to consecutive slots by accident.
static uint64_t mix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

// ============================================================================
// Section 2: probe strategies
// ============================================================================
// A probe strategy is a small policy class with a static function
//     size_t slot(uint64_t hash, size_t attempt, size_t capacity)
// returning the index to inspect on the given attempt (0-based).

struct LinearProbe {
    static constexpr const char* name() { return "linear"; }
    static size_t slot(uint64_t h, size_t i, size_t m) { return (size_t)((mix64(h) + i) % m); }
};

struct DoubleHashProbe {
    static constexpr const char* name() { return "double-hash"; }
    static size_t slot(uint64_t h, size_t i, size_t m) {
        uint64_t h1 = mix64(h);
        // Independent second hash; 1 + (h2 mod (m-1)) is in [1, m-1].
        uint64_t h2 = 1 + mix64(h ^ 0xdeadbeefcafef00dULL) % (m - 1);
        return (size_t)((h1 + i * h2) % m);
    }
};

// ============================================================================
// Section 3: the hash table
// ============================================================================

template <class K, class V, class Probe = LinearProbe, class Hash = std::hash<K>>
class OpenHashTable {
    enum class State : uint8_t { Empty, Occupied, Deleted };

    struct Slot {
        State state = State::Empty;
        K key{};
        V value{};
    };

public:
    // Statistics gathered for experiments.
    struct Stats {
        size_t lookups = 0;      // number of find/insert/erase probes sequences started
        size_t probes = 0;       // total slots inspected
        size_t maxProbe = 0;     // longest probe sequence seen
        size_t rehashes = 0;     // number of rehash operations
    };

    explicit OpenHashTable(size_t initialCapacity = 11, double maxLoad = 0.6)
        : slots_(nextPrime(initialCapacity < 3 ? 3 : initialCapacity)), maxLoad_(maxLoad) {}

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    size_t capacity() const { return slots_.size(); }
    size_t tombstones() const { return deleted_; }
    double loadFactor() const { return (double)size_ / (double)slots_.size(); }
    const Stats& stats() const { return stats_; }
    void resetStats() { stats_ = Stats{}; }

    // Insert or overwrite. Returns true if a new key was added.
    bool insert(const K& key, const V& value) {
        maybeRehash();
        size_t idx;
        if (locate(key, idx)) { // key present: overwrite
            slots_[idx].value = value;
            return false;
        }
        place(key, value);
        return true;
    }

    // Pointer to the value or nullptr.
    V* find(const K& key) {
        size_t idx;
        return locate(key, idx) ? &slots_[idx].value : nullptr;
    }
    const V* find(const K& key) const {
        size_t idx;
        return locate(key, idx) ? &slots_[idx].value : nullptr;
    }

    bool contains(const K& key) const { return find(key) != nullptr; }

    optional<V> get(const K& key) const {
        const V* p = find(key);
        if (p) return *p;
        return nullopt;
    }

    // Access with default-construction, like std::unordered_map.
    V& operator[](const K& key) {
        maybeRehash();
        size_t idx;
        if (locate(key, idx)) return slots_[idx].value;
        return slots_[place(key, V{})].value;
    }

    // Erase by key. Leaves a tombstone so that probe chains stay intact.
    bool erase(const K& key) {
        size_t idx;
        if (!locate(key, idx)) return false;
        slots_[idx].state = State::Deleted;
        slots_[idx].key = K{};
        slots_[idx].value = V{};
        --size_;
        ++deleted_;
        return true;
    }

    // Visit every (key, value) pair in table order.
    void forEach(const function<void(const K&, const V&)>& fn) const {
        for (const Slot& s : slots_)
            if (s.state == State::Occupied) fn(s.key, s.value);
    }

    // Force a rehash into a table with at least newCap slots. Tombstones are
    // dropped, which also shortens probe sequences.
    void rehash(size_t newCap) {
        vector<Slot> old;
        old.swap(slots_);
        slots_.assign(nextPrime(newCap < 3 ? 3 : newCap), Slot{});
        size_ = 0;
        deleted_ = 0;
        for (Slot& s : old)
            if (s.state == State::Occupied) place(s.key, s.value);
        ++stats_.rehashes;
    }

    // Debug helper: render the table as "[idx:key] ..." for small tables.
    string dump() const {
        string out;
        for (size_t i = 0; i < slots_.size(); ++i) {
            out += to_string(i) + ":";
            if (slots_[i].state == State::Empty) out += ".";
            else if (slots_[i].state == State::Deleted) out += "X";
            else out += "o";
            out += i + 1 < slots_.size() ? " " : "";
        }
        return out;
    }

    // Length of the longest run of consecutive non-empty slots (clustering).
    size_t longestCluster() const {
        size_t best = 0, run = 0;
        for (const Slot& s : slots_) {
            if (s.state != State::Empty) best = max(best, ++run);
            else run = 0;
        }
        return best;
    }

    // Average probes per lookup so far.
    double avgProbes() const {
        return stats_.lookups ? (double)stats_.probes / (double)stats_.lookups : 0.0;
    }

private:
    vector<Slot> slots_;
    size_t size_ = 0;     // live entries
    size_t deleted_ = 0;  // tombstones
    double maxLoad_;
    Hash hasher_;
    mutable Stats stats_;

    // Grow (or just clean) when live + tombstones exceed the load threshold.
    void maybeRehash() {
        if ((double)(size_ + deleted_ + 1) / (double)slots_.size() > maxLoad_) {
            // If most of the load is tombstones, rehash at the same size.
            size_t target = ((double)(size_ + 1) / (double)slots_.size() > maxLoad_ / 2)
                                ? slots_.size() * 2 + 1
                                : slots_.size();
            rehash(target);
        }
    }

    // Find the slot holding `key`. Returns false when absent. Stops at the
    // first EMPTY slot; skips over tombstones and non-matching keys.
    bool locate(const K& key, size_t& idxOut) const {
        uint64_t h = (uint64_t)hasher_(key);
        size_t m = slots_.size();
        size_t i = 0;
        ++stats_.lookups;
        for (; i < m; ++i) {
            size_t idx = Probe::slot(h, i, m);
            ++stats_.probes;
            const Slot& s = slots_[idx];
            if (s.state == State::Empty) break;
            if (s.state == State::Occupied && s.key == key) {
                idxOut = idx;
                stats_.maxProbe = max(stats_.maxProbe, i + 1);
                return true;
            }
        }
        stats_.maxProbe = max(stats_.maxProbe, i + 1);
        return false;
    }

    // Place a key known to be absent. Reuses the first tombstone on its path.
    size_t place(const K& key, const V& value) {
        uint64_t h = (uint64_t)hasher_(key);
        size_t m = slots_.size();
        for (size_t i = 0; i < m; ++i) {
            size_t idx = Probe::slot(h, i, m);
            Slot& s = slots_[idx];
            if (s.state != State::Occupied) {
                if (s.state == State::Deleted) --deleted_;
                s.state = State::Occupied;
                s.key = key;
                s.value = value;
                ++size_;
                return idx;
            }
        }
        // Unreachable because maybeRehash keeps the load below 1.
        throw runtime_error("hash table full");
    }
};

// ============================================================================
// Section 4: tests and experiments
// ============================================================================

static void check(bool ok, const string& what) {
    cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << what << "\n";
    assert(ok);
}

template <class Probe>
void testBasics() {
    cout << "=== Basic operations (" << Probe::name() << ") ===\n";
    OpenHashTable<int, string, Probe> t;
    check(t.empty(), "new table is empty");
    check(t.insert(1, "one"), "insert new key returns true");
    check(t.insert(2, "two"), "insert second key");
    check(!t.insert(1, "uno"), "re-insert returns false (overwrite)");
    check(*t.find(1) == "uno", "overwritten value visible");
    check(t.size() == 2, "size is 2");
    check(t.contains(2) && !t.contains(3), "contains works");
    check(t.get(2).value_or("?") == "two", "get() returns optional value");
    check(!t.get(99).has_value(), "get() on missing key is nullopt");
    check(t.erase(1) && !t.erase(1), "erase succeeds once");
    check(!t.contains(1) && t.size() == 1, "erased key gone");
    t[7] = "seven";
    check(t[7] == "seven" && t.size() == 2, "operator[] inserts and returns reference");
    check(t[8].empty() && t.size() == 3, "operator[] default-constructs");
    cout << "\n";
}

template <class Probe>
void testStrings() {
    cout << "=== String keys, word counting (" << Probe::name() << ") ===\n";
    OpenHashTable<string, int, Probe> counts;
    string text = "the quick brown fox jumps over the lazy dog the fox barks and the dog runs";
    string word;
    auto flush = [&]() {
        if (!word.empty()) ++counts[word], word.clear();
    };
    for (char c : text) c == ' ' ? flush() : (void)(word += c);
    flush();
    cout << "  distinct words: " << counts.size() << "\n";
    cout << "  the=" << counts["the"] << " fox=" << counts["fox"] << " dog=" << counts["dog"] << "\n";
    check(counts["the"] == 4 && counts["fox"] == 2 && counts["dog"] == 2, "counts correct");
    check(counts.size() == 11, "11 distinct words");  // incl. "the" etc.
    cout << "\n";
}

template <class Probe>
void testRehash() {
    cout << "=== Rehash behaviour (" << Probe::name() << ") ===\n";
    OpenHashTable<int, int, Probe> t(5, 0.5);
    size_t startCap = t.capacity();
    for (int i = 0; i < 1000; ++i) t.insert(i, i * i);
    cout << "  capacity " << startCap << " -> " << t.capacity() << ", rehashes=" << t.stats().rehashes
         << ", load=" << fixed << setprecision(3) << t.loadFactor() << "\n";
    check(t.size() == 1000, "all 1000 keys present");
    bool ok = true;
    for (int i = 0; i < 1000; ++i) ok = ok && t.find(i) && *t.find(i) == i * i;
    check(ok, "values survive rehash");
    check(t.loadFactor() <= 0.5 + 1e-9, "load factor respected");

    // Manual rehash drops tombstones.
    for (int i = 0; i < 500; ++i) t.erase(i);
    size_t tomb = t.tombstones();
    t.rehash(t.capacity());
    cout << "  tombstones before manual rehash: " << tomb << ", after: " << t.tombstones() << "\n";
    check(tomb == 500 && t.tombstones() == 0 && t.size() == 500, "rehash clears tombstones");
    cout << "\n";
}

template <class Probe>
void testTombstones() {
    cout << "=== Tombstones keep probe chains intact (" << Probe::name() << ") ===\n";
    // Build a table where many keys collide in a small space (high max load).
    OpenHashTable<int, int, Probe> t(23, 0.9);
    for (int i = 0; i < 18; ++i) t.insert(i, i);
    // Erase every other key; the remaining ones must still be found even if
    // their probe path passes through a removed slot.
    for (int i = 0; i < 18; i += 2) t.erase(i);
    bool ok = true;
    for (int i = 1; i < 18; i += 2) ok = ok && t.contains(i);
    for (int i = 0; i < 18; i += 2) ok = ok && !t.contains(i);
    check(ok, "odd keys found, even keys absent after deletions");
    // Re-insert: tombstones should be reused, so size grows but the number of
    // tombstones shrinks.
    size_t before = t.tombstones();
    for (int i = 0; i < 18; i += 2) t.insert(i, -i);
    cout << "  tombstones before reinsertion " << before << ", after " << t.tombstones() << "\n";
    check(t.tombstones() < before, "tombstones are reused");
    check(t.size() == 18, "size restored");
    cout << "  layout: " << t.dump() << "\n\n";
}

// Compare clustering and probe counts between the two strategies at
// increasing load factors. Double hashing should need noticeably fewer
// probes as the table fills up.
void experimentClustering() {
    cout << "=== Experiment: probes per successful lookup vs load factor ===\n";
    cout << "  theory (unsuccessful search): linear ~ 1/2(1+1/(1-a)^2), double ~ 1/(1-a)\n";
    cout << "  " << left << setw(8) << "load" << setw(14) << "linear avg" << setw(14) << "double avg"
         << setw(14) << "linear max" << setw(14) << "double max" << "\n";
    const size_t cap = 10007;
    for (double load : {0.3, 0.5, 0.7, 0.8, 0.9}) {
        OpenHashTable<uint64_t, int, LinearProbe> lin(cap, 0.99);
        OpenHashTable<uint64_t, int, DoubleHashProbe> dbl(cap, 0.99);
        size_t n = (size_t)(load * cap);
        for (size_t i = 0; i < n; ++i) {
            lin.insert(i * 7919 + 13, 1);
            dbl.insert(i * 7919 + 13, 1);
        }
        lin.resetStats();
        dbl.resetStats();
        // Look up keys that are present.
        for (size_t i = 0; i < n; ++i) {
            lin.find(i * 7919 + 13);
            dbl.find(i * 7919 + 13);
        }
        cout << "  " << left << setw(8) << load << setw(14) << fixed << setprecision(3)
             << lin.avgProbes() << setw(14) << dbl.avgProbes() << setw(14) << lin.stats().maxProbe
             << setw(14) << dbl.stats().maxProbe << "\n";
        assert(lin.capacity() == cap && dbl.capacity() == cap); // no rehash occurred
    }
    cout << "\n";
}

template <class Probe>
void testRandomVsStd() {
    cout << "=== Random operations vs std::unordered_map (" << Probe::name() << ") ===\n";
    mt19937 rng(2024);
    OpenHashTable<int, int, Probe> mine(7, 0.65);
    unordered_map<int, int> ref;
    const int OPS = 60000;
    for (int op = 0; op < OPS; ++op) {
        int key = (int)(rng() % 2000);
        int kind = (int)(rng() % 10);
        if (kind < 5) {
            int val = (int)(rng() % 1000000);
            bool added = mine.insert(key, val);
            bool refAdded = ref.find(key) == ref.end();
            ref[key] = val;
            assert(added == refAdded);
        } else if (kind < 8) {
            bool a = mine.erase(key);
            bool b = ref.erase(key) > 0;
            assert(a == b);
        } else {
            const int* p = mine.find(key);
            auto it = ref.find(key);
            assert((p != nullptr) == (it != ref.end()));
            if (p) assert(*p == it->second);
        }
        assert(mine.size() == ref.size());
    }
    // Final full comparison.
    size_t seen = 0;
    bool ok = true;
    mine.forEach([&](const int& k, const int& v) {
        ++seen;
        auto it = ref.find(k);
        ok = ok && it != ref.end() && it->second == v;
    });
    cout << "  final size " << mine.size() << ", capacity " << mine.capacity()
         << ", rehashes " << mine.stats().rehashes << ", avg probes " << fixed << setprecision(2)
         << mine.avgProbes() << "\n";
    check(ok && seen == ref.size(), "contents identical to std::unordered_map after 60000 ops");
    cout << "\n";
}

void testDoubleHashPermutation() {
    cout << "=== Double hashing visits every slot ===\n";
    bool ok = true;
    for (size_t m : {5u, 11u, 101u, 1009u}) {
        for (uint64_t key = 0; key < 200 && ok; ++key) {
            vector<char> seen(m, 0);
            for (size_t i = 0; i < m; ++i) seen[DoubleHashProbe::slot(key, i, m)] = 1;
            for (char c : seen) ok = ok && c;
        }
    }
    check(ok, "probe sequence is a permutation of the slots for prime sizes");

    ok = true;
    for (size_t m : {5u, 11u, 101u}) {
        for (uint64_t key = 0; key < 50; ++key) {
            vector<char> seen(m, 0);
            for (size_t i = 0; i < m; ++i) seen[LinearProbe::slot(key, i, m)] = 1;
            for (char c : seen) ok = ok && c;
        }
    }
    check(ok, "linear probe sequence also covers all slots");
    cout << "\n";
}

void testTinyTableLayout() {
    cout << "=== Tiny table layout (linear, capacity 11, no growth) ===\n";
    OpenHashTable<int, int, LinearProbe> t(11, 0.99);
    for (int k : {10, 22, 31, 4, 15, 28, 17, 88, 59}) t.insert(k, k);
    cout << "  " << t.dump() << "\n";
    cout << "  longest cluster: " << t.longestCluster() << " of " << t.capacity() << " slots\n";
    check(t.size() == 9, "nine keys stored");
    t.erase(22);
    t.erase(4);
    cout << "  after erasing two: " << t.dump() << "\n";
    check(t.tombstones() == 2, "two tombstones (X) appear");
    cout << "\n";
}

int main() {
    testBasics<LinearProbe>();
    testBasics<DoubleHashProbe>();
    testStrings<LinearProbe>();
    testStrings<DoubleHashProbe>();
    testRehash<LinearProbe>();
    testRehash<DoubleHashProbe>();
    testTombstones<LinearProbe>();
    testTombstones<DoubleHashProbe>();
    testDoubleHashPermutation();
    testTinyTableLayout();
    experimentClustering();
    testRandomVsStd<LinearProbe>();
    testRandomVsStd<DoubleHashProbe>();
    cout << "All open-addressing hash table tests passed.\n";
    return 0;
}
