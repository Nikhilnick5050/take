/*
 * ============================================================================
 *  File    : 08_priority_structures.cpp
 *  Topic   : Priority-queue data structures
 *  Standard: C++17
 *  Build   : g++ -std=c++17 -Wall -Wextra 08_priority_structures.cpp -o heaps
 * ----------------------------------------------------------------------------
 *  Contents
 *    1. DAryHeap<T, Compare, D>  - array based d-ary heap (D = 2 is the
 *       classic binary heap), with Floyd's O(n) build-heap and heap sort
 *    2. LeftistHeap              - mergeable heap; every operation is built
 *       on merge(), which runs in O(log n) by walking right spines
 *    3. BinomialHeap             - collection of binomial trees (one per set
 *       bit of n); O(log n) insert / merge / extract-min / decrease-key
 *
 *  Complexity summary (n = number of elements)
 *  +----------------+---------+---------+-----------+----------+----------+
 *  | structure      | push    | top     | pop       | merge    | dec-key  |
 *  +----------------+---------+---------+-----------+----------+----------+
 *  | binary heap    | O(log n)| O(1)    | O(log n)  | O(n)     | O(log n)*|
 *  | leftist heap   | O(log n)| O(1)    | O(log n)  | O(log n) | -        |
 *  | binomial heap  | O(log n)| O(log n)| O(log n)  | O(log n) | O(log n) |
 *  +----------------+---------+---------+-----------+----------+----------+
 *  (*) needs an index map; not implemented for the array heap here.
 *
 *  All three are min-heaps by default (smallest element has top priority);
 *  the array heap accepts any comparator. The pointer based heaps store
 *  ints to keep the code focused on the structure itself.
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// ============================================================================
// Section 1: d-ary / binary heap
// ============================================================================
// Elements live in a vector. For index i (0-based) in a D-ary heap:
//     parent(i)      = (i - 1) / D
//     child(i, k)    = D * i + k + 1      for k = 0..D-1
// Compare(a, b) == true means "a has higher priority than b"; the default
// std::less gives a MIN-heap here because we sift the *smaller* element up.

template <class T, class Compare = less<T>, int D = 2>
class DAryHeap {
    static_assert(D >= 2, "arity must be at least 2");

public:
    DAryHeap() = default;

    // Floyd's heapify: O(n) by sifting down every internal node from the
    // last one up to the root. Cheaper than n pushes (O(n log n)).
    explicit DAryHeap(vector<T> items) : a_(move(items)) {
        if (a_.size() > 1)
            for (size_t i = (a_.size() - 2) / D + 1; i-- > 0;) siftDown(i);
    }

    bool empty() const { return a_.empty(); }
    size_t size() const { return a_.size(); }

    const T& top() const {
        if (a_.empty()) throw out_of_range("top() on empty heap");
        return a_.front();
    }

    void push(T v) {
        a_.push_back(move(v));
        siftUp(a_.size() - 1);
    }

    T pop() {
        if (a_.empty()) throw out_of_range("pop() on empty heap");
        T res = move(a_.front());
        a_.front() = move(a_.back());
        a_.pop_back();
        if (!a_.empty()) siftDown(0);
        return res;
    }

    // Check the heap property for every node (used by tests).
    bool valid() const {
        for (size_t i = 1; i < a_.size(); ++i)
            if (cmp_(a_[i], a_[(i - 1) / D])) return false;
        return true;
    }

private:
    vector<T> a_;
    Compare cmp_;

    void siftUp(size_t i) {
        T v = move(a_[i]);
        while (i > 0) {
            size_t p = (i - 1) / D;
            if (!cmp_(v, a_[p])) break;
            a_[i] = move(a_[p]);
            i = p;
        }
        a_[i] = move(v);
    }

    void siftDown(size_t i) {
        size_t n = a_.size();
        T v = move(a_[i]);
        while (true) {
            size_t first = D * i + 1;
            if (first >= n) break;
            size_t best = first, last = min(first + D, n);
            for (size_t c = first + 1; c < last; ++c)
                if (cmp_(a_[c], a_[best])) best = c;
            if (!cmp_(a_[best], v)) break;
            a_[i] = move(a_[best]);
            i = best;
        }
        a_[i] = move(v);
    }
};

// Heap sort built on the heap above (ascending). O(n log n), not in place.
vector<int> heapSort(const vector<int>& in) {
    DAryHeap<int> h(in);
    vector<int> out;
    out.reserve(in.size());
    while (!h.empty()) out.push_back(h.pop());
    return out;
}

// In-place heap sort with the classic max-heap sift-down on a raw array.
void heapSortInPlace(vector<int>& a) {
    int n = (int)a.size();
    auto sift = [&](int i, int size) {
        while (true) {
            int l = 2 * i + 1, r = l + 1, m = i;
            if (l < size && a[l] > a[m]) m = l;
            if (r < size && a[r] > a[m]) m = r;
            if (m == i) return;
            swap(a[i], a[m]);
            i = m;
        }
    };
    for (int i = n / 2 - 1; i >= 0; --i) sift(i, n); // build max-heap
    for (int end = n - 1; end > 0; --end) {          // move max to the back
        swap(a[0], a[end]);
        sift(0, end);
    }
}

// ============================================================================
// Section 2: Leftist heap
// ============================================================================
// Heap-ordered binary tree with the *leftist property*:
//     npl(left) >= npl(right), where npl = null path length (distance to the
//     nearest missing child; npl(null) = -1 here, a leaf has npl 0).
// Hence the right spine has length <= log2(n+1), and merge only walks right
// spines. Insert = merge with a singleton; pop = merge the two subtrees.

class LeftistHeap {
    struct Node {
        int key;
        int npl = 0;
        Node* left = nullptr;
        Node* right = nullptr;
        explicit Node(int k) : key(k) {}
    };

public:
    LeftistHeap() = default;
    LeftistHeap(const LeftistHeap&) = delete;
    LeftistHeap& operator=(const LeftistHeap&) = delete;
    LeftistHeap(LeftistHeap&& o) noexcept : root_(o.root_), size_(o.size_) {
        o.root_ = nullptr;
        o.size_ = 0;
    }
    ~LeftistHeap() { destroy(root_); }

    bool empty() const { return root_ == nullptr; }
    size_t size() const { return size_; }

    int top() const {
        if (!root_) throw out_of_range("top() on empty leftist heap");
        return root_->key;
    }

    void push(int key) {
        root_ = merge(root_, new Node(key));
        ++size_;
    }

    int pop() {
        if (!root_) throw out_of_range("pop() on empty leftist heap");
        Node* old = root_;
        int k = old->key;
        root_ = merge(old->left, old->right);
        delete old;
        --size_;
        return k;
    }

    // Destructively merge `other` into this heap; `other` becomes empty.
    void meld(LeftistHeap& other) {
        if (this == &other) return;
        root_ = merge(root_, other.root_);
        size_ += other.size_;
        other.root_ = nullptr;
        other.size_ = 0;
    }

    // Verify heap order and the leftist property (used by tests).
    bool valid() const { return check(root_).first; }

    // Length of the right spine (should be O(log n)).
    int rightSpineLength() const {
        int len = 0;
        for (Node* x = root_; x; x = x->right) ++len;
        return len;
    }

private:
    Node* root_ = nullptr;
    size_t size_ = 0;

    static int npl(Node* x) { return x ? x->npl : -1; }

    // The heart of the structure. Recursion depth is bounded by the sum of
    // the two right-spine lengths, i.e. O(log n).
    static Node* merge(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (b->key < a->key) swap(a, b); // a has the smaller root
        a->right = merge(a->right, b);   // merge b into the right subtree
        if (npl(a->left) < npl(a->right)) swap(a->left, a->right); // restore leftist property
        a->npl = npl(a->right) + 1;
        return a;
    }

    // Returns (valid, npl).
    static pair<bool, int> check(Node* x) {
        if (!x) return {true, -1};
        auto l = check(x->left), r = check(x->right);
        bool ok = l.first && r.first && l.second >= r.second && x->npl == r.second + 1;
        if (x->left && x->left->key < x->key) ok = false;
        if (x->right && x->right->key < x->key) ok = false;
        return {ok, x->npl};
    }

    // Iterative destruction: the left spine can be O(n) long, so recursion
    // could overflow the stack on large heaps.
    static void destroy(Node* x) {
        vector<Node*> st;
        if (x) st.push_back(x);
        while (!st.empty()) {
            Node* n = st.back();
            st.pop_back();
            if (n->left) st.push_back(n->left);
            if (n->right) st.push_back(n->right);
            delete n;
        }
    }
};

// ============================================================================
// Section 3: Binomial heap
// ============================================================================
// A binomial tree B_k has 2^k nodes: B_k is two B_{k-1} with one linked below
// the other's root. A binomial heap holds at most one B_k per k, mirroring
// the binary representation of n, kept as a linked "root list" in increasing
// order of degree. Merging two heaps is like binary addition with carries.
//
// Each element gets a stable integer id returned by push(). Nodes may swap
// their (key, id) payloads when bubbling up, so we keep where_[id] updated
// and expose decreaseKey(id, newKey) / erase(id) instead of raw pointers.

class BinomialHeap {
    struct Node {
        int key;
        int id;
        int degree = 0;
        Node* parent = nullptr;
        Node* child = nullptr;   // leftmost child
        Node* sibling = nullptr; // next sibling / next root
        Node(int k, int i) : key(k), id(i) {}
    };

public:
    BinomialHeap() = default;
    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;
    ~BinomialHeap() { destroy(head_); }

    bool empty() const { return head_ == nullptr; }
    size_t size() const { return size_; }

    // Insert key and return its id (valid until the element is removed).
    int push(int key) {
        int id = (int)where_.size();
        Node* n = new Node(key, id);
        where_.push_back(n);
        head_ = unite(head_, n);
        ++size_;
        return id;
    }

    int top() const {
        if (!head_) throw out_of_range("top() on empty binomial heap");
        return minRoot()->key;
    }

    // Remove and return the minimum key.
    int pop() {
        if (!head_) throw out_of_range("pop() on empty binomial heap");
        return extract(minRoot());
    }

    // Lower the key of element `id`. newKey must not exceed its current key.
    void decreaseKey(int id, int newKey) {
        Node* x = nodeOf(id);
        if (newKey > x->key) throw invalid_argument("decreaseKey: new key is larger");
        x->key = newKey;
        bubbleUp(x, false);
    }

    // Remove an arbitrary element by id. Returns its key.
    int erase(int id) {
        Node* x = nodeOf(id);
        int k = x->key;
        bubbleUp(x, true); // force it to the root of its tree
        // After bubbling, the element with this id sits at a root.
        Node* r = nodeOf(id);
        extract(r);
        return k;
    }

    // Merge another heap into this one. `other` is left empty. Ids of the
    // other heap are NOT transferable, so erase/decreaseKey must only be
    // used with ids of elements pushed directly into this heap.
    void meld(BinomialHeap& other) {
        if (this == &other) return;
        // Re-register the other heap's nodes under fresh ids in this heap.
        vector<Node*> stack;
        for (Node* r = other.head_; r; r = r->sibling) stack.push_back(r);
        while (!stack.empty()) {
            Node* n = stack.back();
            stack.pop_back();
            for (Node* c = n->child; c; c = c->sibling) stack.push_back(c);
            n->id = (int)where_.size();
            where_.push_back(n);
        }
        head_ = unite(head_, other.head_);
        size_ += other.size_;
        other.head_ = nullptr;
        other.size_ = 0;
        other.where_.clear();
    }

    // Structure checks: degrees strictly increase along the root list, every
    // tree B_k has 2^k nodes and is heap ordered. Also size == popcount sum.
    bool valid() const {
        int prevDeg = -1;
        size_t total = 0;
        for (Node* r = head_; r; r = r->sibling) {
            if (r->degree <= prevDeg || r->parent) return false;
            prevDeg = r->degree;
            size_t cnt = 0;
            if (!checkTree(r, cnt)) return false;
            if (cnt != ((size_t)1 << r->degree)) return false;
            total += cnt;
        }
        return total == size_;
    }

    // Degrees of the roots, e.g. n = 13 = 0b1101 -> {0, 2, 3}.
    vector<int> rootDegrees() const {
        vector<int> d;
        for (Node* r = head_; r; r = r->sibling) d.push_back(r->degree);
        return d;
    }

private:
    Node* head_ = nullptr;
    size_t size_ = 0;
    vector<Node*> where_; // id -> node currently holding it (nullptr if removed)

    Node* nodeOf(int id) const {
        if (id < 0 || id >= (int)where_.size() || !where_[id])
            throw invalid_argument("unknown or removed id");
        return where_[id];
    }

    Node* minRoot() const {
        Node* best = head_;
        for (Node* r = head_->sibling; r; r = r->sibling)
            if (r->key < best->key) best = r;
        return best;
    }

    // Make `child` the leftmost child of `parent` (both roots of B_{k-1}).
    static void link(Node* child, Node* parent) {
        child->parent = parent;
        child->sibling = parent->child;
        parent->child = child;
        ++parent->degree;
    }

    // Merge two root lists into one ordered by degree (no linking yet).
    static Node* mergeRootLists(Node* a, Node* b) {
        Node dummy(0, -1);
        Node* tail = &dummy;
        while (a && b) {
            if (a->degree <= b->degree) {
                tail->sibling = a;
                a = a->sibling;
            } else {
                tail->sibling = b;
                b = b->sibling;
            }
            tail = tail->sibling;
        }
        tail->sibling = a ? a : b;
        return dummy.sibling;
    }

    // Binary-addition style union of two heaps given by root lists.
    static Node* unite(Node* h1, Node* h2) {
        Node* head = mergeRootLists(h1, h2);
        if (!head) return nullptr;
        Node* prev = nullptr;
        Node* x = head;
        Node* next = x->sibling;
        while (next) {
            bool skip = x->degree != next->degree ||
                        (next->sibling && next->sibling->degree == x->degree);
            if (skip) { // case 1 & 2: nothing to link here (or wait for the later pair)
                prev = x;
                x = next;
            } else if (x->key <= next->key) { // case 3: x becomes parent of next
                x->sibling = next->sibling;
                link(next, x);
            } else { // case 4: next becomes parent of x
                if (!prev) head = next;
                else prev->sibling = next;
                link(x, next);
                x = next;
            }
            next = x->sibling;
        }
        return head;
    }

    // Swap payloads with parents while smaller than them (or unconditionally
    // up to the root when `force` is set, used by erase()).
    void bubbleUp(Node* x, bool force) {
        while (x->parent && (force || x->key < x->parent->key)) {
            Node* p = x->parent;
            swap(x->key, p->key);
            swap(x->id, p->id);
            where_[x->id] = x;
            where_[p->id] = p;
            x = p;
        }
    }

    // Remove root r from the root list and merge its children back in.
    int extract(Node* r) {
        int k = r->key;
        // Unlink r from the root list.
        if (head_ == r) head_ = r->sibling;
        else {
            Node* p = head_;
            while (p->sibling != r) p = p->sibling;
            p->sibling = r->sibling;
        }
        // The children list is in decreasing degree order; reverse it.
        Node* rev = nullptr;
        Node* c = r->child;
        while (c) {
            Node* nx = c->sibling;
            c->sibling = rev;
            c->parent = nullptr;
            rev = c;
            c = nx;
        }
        head_ = unite(head_, rev);
        where_[r->id] = nullptr;
        delete r;
        --size_;
        return k;
    }

    static bool checkTree(Node* x, size_t& count) {
        ++count;
        int expectedDeg = x->degree;
        int seen = 0;
        // Children must have degrees degree-1, degree-2, ..., 0 from left to right.
        for (Node* c = x->child; c; c = c->sibling) {
            if (c->parent != x || c->key < x->key) return false;
            if (c->degree != expectedDeg - 1 - seen) return false;
            if (!checkTree(c, count)) return false;
            ++seen;
        }
        return seen == expectedDeg;
    }

    static void destroy(Node* root) {
        vector<Node*> st;
        for (Node* r = root; r; r = r->sibling) st.push_back(r);
        while (!st.empty()) {
            Node* n = st.back();
            st.pop_back();
            for (Node* c = n->child; c; c = c->sibling) st.push_back(c);
            delete n;
        }
    }
};

// ============================================================================
// Section 4: tests
// ============================================================================

static void check(bool ok, const string& what) {
    cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << what << "\n";
    assert(ok);
}

static vector<int> randomVec(mt19937& rng, size_t n, int range) {
    vector<int> v(n);
    for (int& x : v) x = (int)(rng() % (unsigned)range) - range / 2;
    return v;
}

template <int D>
void testDAry(mt19937& rng) {
    cout << "  -- arity " << D << " --\n";
    DAryHeap<int, less<int>, D> h;
    bool ok = true;
    for (int x : randomVec(rng, 500, 1000)) {
        h.push(x);
        ok = ok && h.valid();
    }
    check(ok && h.size() == 500, "500 pushes keep heap property");
    int prev = INT_MIN;
    ok = true;
    while (!h.empty()) {
        int x = h.pop();
        ok = ok && x >= prev;
        prev = x;
    }
    check(ok, "pops come out in ascending order");

    // Floyd build-heap
    auto v = randomVec(rng, 1000, 5000);
    DAryHeap<int, less<int>, D> built(v);
    check(built.valid(), "Floyd heapify produces a valid heap");
    sort(v.begin(), v.end());
    ok = true;
    for (int x : v) ok = ok && built.pop() == x;
    check(ok, "heapified contents sorted correctly");
}

void testBinaryHeap() {
    cout << "=== Part 1: array heap ===\n";
    mt19937 rng(1);
    testDAry<2>(rng);
    testDAry<3>(rng);
    testDAry<4>(rng);

    // Max-heap by comparator, and a heap of pairs.
    DAryHeap<int, greater<int>> mx;
    for (int x : {5, 1, 9, 3, 7}) mx.push(x);
    check(mx.top() == 9, "max-heap via greater<> has 9 on top");
    DAryHeap<pair<int, string>, less<pair<int, string>>> pq;
    pq.push({3, "c"});
    pq.push({1, "a"});
    pq.push({2, "b"});
    check(pq.pop().second == "a" && pq.pop().second == "b", "heap of pairs orders by first");

    bool threw = false;
    try {
        DAryHeap<int> e;
        e.pop();
    } catch (const out_of_range&) { threw = true; }
    check(threw, "pop on empty throws out_of_range");

    auto data = randomVec(rng, 2000, 100000);
    auto expect = data;
    sort(expect.begin(), expect.end());
    check(heapSort(data) == expect, "heapSort matches std::sort");
    auto copy = data;
    heapSortInPlace(copy);
    check(copy == expect, "in-place heap sort matches std::sort");

    // Randomised comparison with std::priority_queue.
    DAryHeap<int> mine;
    priority_queue<int, vector<int>, greater<int>> ref;
    bool ok = true;
    for (int i = 0; i < 20000; ++i) {
        if (ref.empty() || rng() % 3) {
            int x = (int)(rng() % 10000);
            mine.push(x);
            ref.push(x);
        } else {
            ok = ok && mine.pop() == ref.top();
            ref.pop();
        }
        ok = ok && mine.size() == ref.size();
    }
    check(ok, "20000 random ops agree with std::priority_queue");
    cout << "\n";
}

void testLeftist() {
    cout << "=== Part 2: leftist heap ===\n";
    mt19937 rng(2);
    LeftistHeap h;
    check(h.empty(), "new heap is empty");
    for (int x : {5, 3, 8, 1, 9, 2}) h.push(x);
    check(h.top() == 1 && h.size() == 6 && h.valid(), "after 6 pushes, min is 1");

    LeftistHeap a, b;
    vector<int> all;
    for (int x : randomVec(rng, 300, 1000)) a.push(x), all.push_back(x);
    for (int x : randomVec(rng, 400, 1000)) b.push(x), all.push_back(x);
    a.meld(b);
    check(b.empty() && a.size() == 700 && a.valid(), "meld empties the other heap");
    sort(all.begin(), all.end());
    bool ok = true;
    for (int x : all) ok = ok && a.pop() == x;
    check(ok && a.empty(), "merged heap pops in sorted order");

    // Right spine stays logarithmic even for sorted insertions.
    LeftistHeap s;
    const int N = 100000;
    for (int i = 0; i < N; ++i) s.push(i);
    int spine = s.rightSpineLength();
    cout << "  right spine after " << N << " ascending pushes: " << spine << "\n";
    check(spine <= 18, "right spine <= log2(n+1) (about 17)");
    check(s.valid(), "structure valid after 100000 pushes");

    LeftistHeap d;
    for (int i = 0; i < 20000; ++i) d.push(100000 - i); // decreasing: long left spine
    check(d.top() == 80001, "decreasing inserts handled");

    // Random mixed operations vs std::priority_queue.
    LeftistHeap mine;
    priority_queue<int, vector<int>, greater<int>> ref;
    ok = true;
    for (int i = 0; i < 20000; ++i) {
        if (ref.empty() || rng() % 3) {
            int x = (int)(rng() % 5000);
            mine.push(x);
            ref.push(x);
        } else {
            ok = ok && mine.pop() == ref.top();
            ref.pop();
        }
    }
    check(ok && mine.valid(), "20000 random ops agree with std::priority_queue");

    bool threw = false;
    try {
        LeftistHeap e;
        e.top();
    } catch (const out_of_range&) { threw = true; }
    check(threw, "top on empty throws");
    cout << "\n";
}

void testBinomial() {
    cout << "=== Part 3: binomial heap ===\n";
    mt19937 rng(3);
    BinomialHeap h;
    for (int i = 1; i <= 13; ++i) h.push(100 - i);
    auto deg = h.rootDegrees();
    cout << "  n=13 (0b1101) root degrees:";
    for (int d : deg) cout << " " << d;
    cout << "\n";
    check((deg == vector<int>{0, 2, 3}), "root degrees mirror the binary form of n");
    check(h.valid() && h.top() == 87, "valid heap, min = 87");

    bool ok = true;
    for (int n = 1; n <= 64; ++n) {
        BinomialHeap t;
        for (int i = 0; i < n; ++i) t.push((int)(rng() % 1000));
        auto d = t.rootDegrees();
        size_t sum = 0;
        for (int x : d) sum += (size_t)1 << x;
        ok = ok && sum == (size_t)n && t.valid() && (int)d.size() == __builtin_popcount(n);
    }
    check(ok, "for n = 1..64 trees correspond to set bits of n");

    // Pops sorted
    BinomialHeap p;
    vector<int> vals = randomVec(rng, 1000, 5000);
    for (int x : vals) p.push(x);
    sort(vals.begin(), vals.end());
    ok = true;
    for (int x : vals) {
        ok = ok && p.pop() == x;
    }
    check(ok && p.empty() && p.valid(), "1000 pops are sorted");

    // Meld
    BinomialHeap a, b;
    vector<int> all;
    for (int x : randomVec(rng, 123, 1000)) a.push(x), all.push_back(x);
    for (int x : randomVec(rng, 200, 1000)) b.push(x), all.push_back(x);
    a.meld(b);
    check(a.size() == 323 && b.empty() && a.valid(), "meld of 123 + 200 elements");
    sort(all.begin(), all.end());
    ok = true;
    for (int x : all) ok = ok && a.pop() == x;
    check(ok, "melded heap pops sorted");

    // decreaseKey and erase
    BinomialHeap k;
    vector<int> ids;
    for (int i = 0; i < 50; ++i) ids.push_back(k.push(1000 + i));
    k.decreaseKey(ids[37], 5);
    check(k.top() == 5, "decreaseKey makes element the minimum");
    k.decreaseKey(ids[10], 1);
    check(k.top() == 1 && k.valid(), "second decreaseKey");
    int removed = k.erase(ids[10]);
    check(removed == 1 && k.top() == 5 && k.size() == 49 && k.valid(), "erase by id");
    removed = k.erase(ids[49]);
    check(removed == 1049 && k.size() == 48 && k.valid(), "erase a large element");
    bool threw = false;
    try {
        k.decreaseKey(ids[10], 0); // already removed
    } catch (const invalid_argument&) { threw = true; }
    check(threw, "using a removed id throws");
    threw = false;
    try {
        k.decreaseKey(ids[0], 5000);
    } catch (const invalid_argument&) { threw = true; }
    check(threw, "increasing via decreaseKey throws");

    // Randomised against multiset-like reference.
    BinomialHeap mine;
    priority_queue<int, vector<int>, greater<int>> ref;
    ok = true;
    for (int i = 0; i < 20000; ++i) {
        if (ref.empty() || rng() % 3) {
            int x = (int)(rng() % 5000);
            mine.push(x);
            ref.push(x);
        } else {
            ok = ok && mine.pop() == ref.top();
            ref.pop();
        }
        if (i % 2000 == 0) ok = ok && mine.valid();
    }
    check(ok && mine.valid(), "20000 random ops agree with std::priority_queue");
    cout << "\n";
}

// Use all three structures to solve the same task (k smallest of a stream)
// and show that they agree.
void testAgreement() {
    cout << "=== Part 4: all heaps agree on a merge-and-drain task ===\n";
    mt19937 rng(4);
    auto data = randomVec(rng, 5000, 1000000);
    DAryHeap<int> bin;
    LeftistHeap lef;
    BinomialHeap bio;
    for (int x : data) {
        bin.push(x);
        lef.push(x);
        bio.push(x);
    }
    bool ok = true;
    for (int i = 0; i < 5000; ++i) {
        int a = bin.pop(), b = lef.pop(), c = bio.pop();
        ok = ok && a == b && b == c;
    }
    check(ok && bin.empty() && lef.empty() && bio.empty(), "5000 pops identical across the three heaps");
    cout << "\n";
}

int main() {
    testBinaryHeap();
    testLeftist();
    testBinomial();
    testAgreement();
    cout << "All priority-structure tests passed.\n";
    return 0;
}
