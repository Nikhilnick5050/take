/*
 * ============================================================================
 *  01_avl_tree.cpp
 * ============================================================================
 *  AVL tree (self-balancing binary search tree) with order statistics.
 *
 *  Features:
 *    - insert / erase (unique keys)
 *    - single and double rotations (LL, RR, LR, RL) via rebalance()
 *    - subtree sizes for order statistics:
 *        kth(k)          : k-th smallest element (0-based)
 *        rank(x)         : number of elements strictly smaller than x
 *        lower_bound(x)  : smallest element >= x
 *        upper_bound(x)  : smallest element >  x
 *        count_range(l,r): number of elements in [l, r]
 *    - invariant validation (balance factor, heights, sizes, BST order)
 *    - randomized stress test compared against std::set
 *
 *  Complexity: every operation is O(log n) worst case.
 *
 *  Build:  g++ -std=c++17 -Wall -Wextra -O2 01_avl_tree.cpp -o avl
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
//  AVLTree<T>
// ---------------------------------------------------------------------------
template <typename T, typename Compare = std::less<T>>
class AVLTree {
    // A node stores its subtree height and size so that balance factors and
    // order statistics can be computed in O(1) per node.
    struct Node {
        T key;
        int height = 1;     // height of the subtree rooted here (leaf = 1)
        int size = 1;       // number of nodes in the subtree rooted here
        Node* left = nullptr;
        Node* right = nullptr;
        explicit Node(const T& k) : key(k) {}
    };

    Node* root_ = nullptr;
    Compare cmp_;
    int rotations_ = 0;  // statistic: how many rotations were performed

public:
    AVLTree() = default;
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    ~AVLTree() { destroy(root_); }

    // ----- basic queries ---------------------------------------------------
    int size() const { return sz(root_); }
    bool empty() const { return root_ == nullptr; }
    int height() const { return ht(root_); }
    int rotations() const { return rotations_; }

    bool contains(const T& key) const {
        Node* cur = root_;
        while (cur) {
            if (cmp_(key, cur->key)) cur = cur->left;
            else if (cmp_(cur->key, key)) cur = cur->right;
            else return true;
        }
        return false;
    }

    // ----- modification ----------------------------------------------------
    // Returns true if the key was inserted, false if it already existed.
    bool insert(const T& key) {
        bool inserted = false;
        root_ = insert(root_, key, inserted);
        return inserted;
    }

    // Returns true if the key existed and was removed.
    bool erase(const T& key) {
        bool erased = false;
        root_ = erase(root_, key, erased);
        return erased;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
    }

    // ----- order statistics ------------------------------------------------
    // k-th smallest element, 0-based. Empty optional if k is out of range.
    std::optional<T> kth(int k) const {
        if (k < 0 || k >= size()) return std::nullopt;
        Node* cur = root_;
        while (cur) {
            int leftSize = sz(cur->left);
            if (k < leftSize) {
                cur = cur->left;
            } else if (k == leftSize) {
                return cur->key;
            } else {
                k -= leftSize + 1;
                cur = cur->right;
            }
        }
        return std::nullopt;  // unreachable when sizes are consistent
    }

    // Number of stored elements strictly less than key.
    int rank(const T& key) const {
        int r = 0;
        Node* cur = root_;
        while (cur) {
            if (cmp_(cur->key, key)) {
                r += sz(cur->left) + 1;
                cur = cur->right;
            } else {
                cur = cur->left;
            }
        }
        return r;
    }

    // Smallest element >= key.
    std::optional<T> lower_bound(const T& key) const {
        std::optional<T> best;
        Node* cur = root_;
        while (cur) {
            if (!cmp_(cur->key, key)) {  // cur->key >= key
                best = cur->key;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        return best;
    }

    // Smallest element > key.
    std::optional<T> upper_bound(const T& key) const {
        std::optional<T> best;
        Node* cur = root_;
        while (cur) {
            if (cmp_(key, cur->key)) {  // cur->key > key
                best = cur->key;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        return best;
    }

    // Number of elements x with lo <= x <= hi.
    int count_range(const T& lo, const T& hi) const {
        if (cmp_(hi, lo)) return 0;
        // elements <= hi  minus elements < lo
        return rank_le(hi) - rank(lo);
    }

    std::optional<T> min() const {
        if (!root_) return std::nullopt;
        Node* c = root_;
        while (c->left) c = c->left;
        return c->key;
    }
    std::optional<T> max() const {
        if (!root_) return std::nullopt;
        Node* c = root_;
        while (c->right) c = c->right;
        return c->key;
    }

    // ----- traversal / debugging ------------------------------------------
    std::vector<T> inorder() const {
        std::vector<T> out;
        out.reserve(size());
        inorder(root_, out);
        return out;
    }

    // Parenthesised structure, e.g. "(( 1 ) 2 ( 3 ))".
    std::string toString() const {
        std::ostringstream os;
        print(root_, os);
        return os.str();
    }

    // Verify every AVL invariant. Returns true when the tree is valid.
    bool validate() const {
        int h = 0, s = 0;
        return validate(root_, nullptr, nullptr, h, s);
    }

private:
    // ----- tiny helpers ----------------------------------------------------
    static int ht(const Node* n) { return n ? n->height : 0; }
    static int sz(const Node* n) { return n ? n->size : 0; }
    static int balanceFactor(const Node* n) {
        return n ? ht(n->left) - ht(n->right) : 0;
    }
    // Recompute cached height and size from the children.
    static void update(Node* n) {
        n->height = 1 + std::max(ht(n->left), ht(n->right));
        n->size = 1 + sz(n->left) + sz(n->right);
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    int rank_le(const T& key) const {
        int r = 0;
        Node* cur = root_;
        while (cur) {
            if (!cmp_(key, cur->key)) {  // cur->key <= key
                r += sz(cur->left) + 1;
                cur = cur->right;
            } else {
                cur = cur->left;
            }
        }
        return r;
    }

    // ----- rotations -------------------------------------------------------
    //        y                x
    //       / \              / \  (diagram)
    //      x   C   ---->    A   y
    //     / \                  / \  (diagram)
    //    A   B                B   C
    Node* rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        update(y);  // y is now the lower node: update it first
        update(x);
        ++rotations_;
        return x;
    }

    //      x                    y
    //     / \                  / \  (diagram)
    //    A   y      ---->     x   C
    //       / \              / \  (diagram)
    //      B   C            A   B
    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        update(x);
        update(y);
        ++rotations_;
        return y;
    }

    // Restore the AVL property at n (children are already valid AVL trees).
    Node* rebalance(Node* n) {
        update(n);
        int bf = balanceFactor(n);
        if (bf > 1) {                       // left heavy
            if (balanceFactor(n->left) < 0)  // LR case: first rotate child left
                n->left = rotateLeft(n->left);
            return rotateRight(n);           // LL case
        }
        if (bf < -1) {                      // right heavy
            if (balanceFactor(n->right) > 0)  // RL case
                n->right = rotateRight(n->right);
            return rotateLeft(n);            // RR case
        }
        return n;
    }

    // ----- recursive insert / erase ---------------------------------------
    Node* insert(Node* n, const T& key, bool& inserted) {
        if (!n) {
            inserted = true;
            return new Node(key);
        }
        if (cmp_(key, n->key)) n->left = insert(n->left, key, inserted);
        else if (cmp_(n->key, key)) n->right = insert(n->right, key, inserted);
        else return n;  // duplicate: nothing changes
        return rebalance(n);
    }

    // Detach the minimum node of the subtree; returns new subtree root and
    // stores the detached node in `minNode`.
    Node* detachMin(Node* n, Node*& minNode) {
        if (!n->left) {
            minNode = n;
            return n->right;
        }
        n->left = detachMin(n->left, minNode);
        return rebalance(n);
    }

    Node* erase(Node* n, const T& key, bool& erased) {
        if (!n) return nullptr;
        if (cmp_(key, n->key)) {
            n->left = erase(n->left, key, erased);
        } else if (cmp_(n->key, key)) {
            n->right = erase(n->right, key, erased);
        } else {
            erased = true;
            Node* l = n->left;
            Node* r = n->right;
            if (!l || !r) {  // zero or one child
                delete n;
                return l ? l : r;
            }
            // Two children: replace by in-order successor.
            Node* succ = nullptr;
            Node* newRight = detachMin(r, succ);
            succ->left = l;
            succ->right = newRight;
            delete n;
            n = succ;
        }
        return rebalance(n);
    }

    // ----- traversal helpers ----------------------------------------------
    static void inorder(const Node* n, std::vector<T>& out) {
        if (!n) return;
        inorder(n->left, out);
        out.push_back(n->key);
        inorder(n->right, out);
    }

    static void print(const Node* n, std::ostringstream& os) {
        if (!n) return;
        if (!n->left && !n->right) {
            os << n->key;
            return;
        }
        os << "(";
        if (n->left) print(n->left, os); else os << ".";
        os << " " << n->key << " ";
        if (n->right) print(n->right, os); else os << ".";
        os << ")";
    }

    bool validate(const Node* n, const T* lo, const T* hi, int& h, int& s) const {
        if (!n) {
            h = 0;
            s = 0;
            return true;
        }
        // BST ordering within (lo, hi)
        if (lo && !cmp_(*lo, n->key)) return false;
        if (hi && !cmp_(n->key, *hi)) return false;
        int lh, ls, rh, rs;
        if (!validate(n->left, lo, &n->key, lh, ls)) return false;
        if (!validate(n->right, &n->key, hi, rh, rs)) return false;
        h = 1 + std::max(lh, rh);
        s = 1 + ls + rs;
        if (std::abs(lh - rh) > 1) return false;  // balance
        if (h != n->height) return false;         // cached height
        if (s != n->size) return false;           // cached size
        return true;
    }
};

// ---------------------------------------------------------------------------
//  Test helpers
// ---------------------------------------------------------------------------
static int g_failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            ++g_failures;                                                    \
            std::cout << "  [FAIL] line " << __LINE__ << ": " #cond "\n";    \
        }                                                                    \
    } while (0)

template <typename T>
static std::string join(const std::vector<T>& v) {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) os << (i ? ", " : "") << v[i];
    os << "]";
    return os.str();
}

// ---------------------------------------------------------------------------
//  Tests
// ---------------------------------------------------------------------------
static void testRotations() {
    std::cout << "== Rotation tests ==\n";

    {   // Right-right case: ascending inserts force a left rotation.
        AVLTree<int> t;
        for (int x : {1, 2, 3}) t.insert(x);
        std::cout << "  RR (1,2,3)      : " << t.toString()
                  << " rotations=" << t.rotations() << "\n";
        CHECK(t.toString() == "(1 2 3)");
        CHECK(t.rotations() == 1);
    }
    {   // Left-left case.
        AVLTree<int> t;
        for (int x : {3, 2, 1}) t.insert(x);
        std::cout << "  LL (3,2,1)      : " << t.toString() << "\n";
        CHECK(t.toString() == "(1 2 3)");
    }
    {   // Left-right case: needs a double rotation (2 rotations).
        AVLTree<int> t;
        for (int x : {3, 1, 2}) t.insert(x);
        std::cout << "  LR (3,1,2)      : " << t.toString()
                  << " rotations=" << t.rotations() << "\n";
        CHECK(t.toString() == "(1 2 3)");
        CHECK(t.rotations() == 2);
    }
    {   // Right-left case.
        AVLTree<int> t;
        for (int x : {1, 3, 2}) t.insert(x);
        std::cout << "  RL (1,3,2)      : " << t.toString()
                  << " rotations=" << t.rotations() << "\n";
        CHECK(t.toString() == "(1 2 3)");
        CHECK(t.rotations() == 2);
    }
    {   // Sequential inserts of 1..1023 must give a perfect tree of height 10.
        AVLTree<int> t;
        for (int i = 1; i <= 1023; ++i) t.insert(i);
        std::cout << "  1..1023 height  : " << t.height() << "\n";
        CHECK(t.height() == 10);
        CHECK(t.validate());
    }
}

static void testBasicOps() {
    std::cout << "== Basic insert/erase/search ==\n";
    AVLTree<int> t;
    CHECK(t.empty());
    std::vector<int> vals = {50, 20, 70, 10, 30, 60, 80, 25, 35, 65};
    for (int v : vals) CHECK(t.insert(v));
    CHECK(!t.insert(30));  // duplicate rejected
    CHECK(t.size() == 10);
    CHECK(t.validate());
    std::cout << "  inorder         : " << join(t.inorder()) << "\n";
    std::cout << "  structure       : " << t.toString() << "\n";
    CHECK(t.contains(25));
    CHECK(!t.contains(26));

    // Erase a leaf, a node with one child, a node with two children, the root.
    CHECK(t.erase(10));
    CHECK(t.erase(80));
    CHECK(t.erase(20));
    CHECK(t.erase(50));
    CHECK(!t.erase(999));
    CHECK(t.validate());
    std::cout << "  after erasures  : " << join(t.inorder()) << "\n";
    CHECK(t.inorder() == (std::vector<int>{25, 30, 35, 60, 65, 70}));
    CHECK(t.min().value() == 25);
    CHECK(t.max().value() == 70);

    t.clear();
    CHECK(t.empty());
    CHECK(!t.min().has_value());
}

static void testOrderStatistics() {
    std::cout << "== Order statistics ==\n";
    AVLTree<int> t;
    for (int i = 0; i < 100; ++i) t.insert(i * 3);  // 0,3,6,...,297

    CHECK(t.kth(0).value() == 0);
    CHECK(t.kth(1).value() == 3);
    CHECK(t.kth(99).value() == 297);
    CHECK(!t.kth(100).has_value());
    CHECK(!t.kth(-1).has_value());
    std::cout << "  kth(0,1,50,99)  : " << *t.kth(0) << " " << *t.kth(1) << " "
              << *t.kth(50) << " " << *t.kth(99) << "\n";

    CHECK(t.rank(0) == 0);
    CHECK(t.rank(1) == 1);
    CHECK(t.rank(3) == 1);
    CHECK(t.rank(4) == 2);
    CHECK(t.rank(1000) == 100);
    std::cout << "  rank(4,150,1000): " << t.rank(4) << " " << t.rank(150)
              << " " << t.rank(1000) << "\n";

    CHECK(t.lower_bound(4).value() == 6);
    CHECK(t.lower_bound(6).value() == 6);
    CHECK(t.upper_bound(6).value() == 9);
    CHECK(!t.lower_bound(298).has_value());
    CHECK(!t.upper_bound(297).has_value());
    std::cout << "  lb(4) ub(6)     : " << *t.lower_bound(4) << " "
              << *t.upper_bound(6) << "\n";

    CHECK(t.count_range(0, 297) == 100);
    CHECK(t.count_range(10, 20) == 3);  // 12, 15, 18
    CHECK(t.count_range(12, 18) == 3);
    CHECK(t.count_range(20, 10) == 0);
    std::cout << "  count[10,20]    : " << t.count_range(10, 20) << "\n";

    // kth and rank are inverse operations.
    bool inverse = true;
    for (int k = 0; k < t.size(); ++k)
        if (t.rank(*t.kth(k)) != k) inverse = false;
    CHECK(inverse);
}

static void testStringsAndComparator() {
    std::cout << "== Strings and custom comparator ==\n";
    AVLTree<std::string> words;
    for (const char* w : {"pear", "apple", "fig", "banana", "cherry", "date"})
        words.insert(w);
    std::cout << "  sorted words    : " << join(words.inorder()) << "\n";
    CHECK(words.kth(0).value() == "apple");
    CHECK(words.lower_bound("c").value() == "cherry");

    AVLTree<int, std::greater<int>> desc;  // descending order
    for (int i = 1; i <= 10; ++i) desc.insert(i);
    std::cout << "  descending      : " << join(desc.inorder()) << "\n";
    CHECK(desc.kth(0).value() == 10);
    CHECK(desc.validate());
}

static void testStress() {
    std::cout << "== Randomized stress vs std::set ==\n";
    std::mt19937 rng(12345);
    AVLTree<int> t;
    std::set<int> ref;
    const int OPS = 60000;
    const int RANGE = 2000;
    int mismatches = 0;

    for (int i = 0; i < OPS; ++i) {
        int op = static_cast<int>(rng() % 3);
        int x = static_cast<int>(rng() % RANGE);
        if (op == 0 || op == 1) {
            bool a = t.insert(x);
            bool b = ref.insert(x).second;
            if (a != b) ++mismatches;
        } else {
            bool a = t.erase(x);
            bool b = ref.erase(x) > 0;
            if (a != b) ++mismatches;
        }
        // Periodically check the full invariants and order statistics.
        if (i % 500 == 0) {
            if (!t.validate()) ++mismatches;
            if (t.size() != static_cast<int>(ref.size())) ++mismatches;
            int y = static_cast<int>(rng() % RANGE);
            int expectedRank = static_cast<int>(
                std::distance(ref.begin(), ref.lower_bound(y)));
            if (t.rank(y) != expectedRank) ++mismatches;
            if (!ref.empty()) {
                int k = static_cast<int>(rng() % ref.size());
                auto it = ref.begin();
                std::advance(it, k);
                if (t.kth(k).value() != *it) ++mismatches;
            }
            auto lb = ref.lower_bound(y);
            auto mine = t.lower_bound(y);
            if ((lb == ref.end()) != !mine.has_value()) ++mismatches;
            else if (lb != ref.end() && *lb != *mine) ++mismatches;
        }
    }
    std::vector<int> expected(ref.begin(), ref.end());
    CHECK(t.inorder() == expected);
    CHECK(t.validate());
    CHECK(mismatches == 0);
    std::cout << "  ops=" << OPS << " final size=" << t.size()
              << " height=" << t.height() << " rotations=" << t.rotations()
              << " mismatches=" << mismatches << "\n";
    // Height of an AVL tree is < 1.4405 * log2(n + 2).
    CHECK(t.height() <= 1.45 * std::log2(t.size() + 2.0) + 1);
}

static void testEraseEverything() {
    std::cout << "== Erase everything in random order ==\n";
    std::vector<int> v(5000);
    for (int i = 0; i < 5000; ++i) v[i] = i;
    std::mt19937 rng(7);
    std::shuffle(v.begin(), v.end(), rng);
    AVLTree<int> t;
    for (int x : v) t.insert(x);
    CHECK(t.size() == 5000);
    std::shuffle(v.begin(), v.end(), rng);
    bool ok = true;
    for (size_t i = 0; i < v.size(); ++i) {
        if (!t.erase(v[i])) ok = false;
        if (i % 250 == 0 && !t.validate()) ok = false;
    }
    CHECK(ok);
    CHECK(t.empty());
    std::cout << "  tree empty      : " << std::boolalpha << t.empty() << "\n";
}

int main() {
    std::cout << "AVL tree with order statistics\n\n";
    testRotations();
    testBasicOps();
    testOrderStatistics();
    testStringsAndComparator();
    testStress();
    testEraseEverything();
    std::cout << "\n" << (g_failures == 0 ? "ALL TESTS PASSED"
                                          : "SOME TESTS FAILED")
              << " (failures: " << g_failures << ")\n";
    return g_failures == 0 ? 0 : 1;
}
