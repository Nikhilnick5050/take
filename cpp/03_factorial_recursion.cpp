// 03_factorial_recursion.cpp
// Two ways to compute n! (factorial): recursion and a simple loop.
// Compile: g++ -std=c++17 -Wall -o /tmp/x cpp/03_factorial_recursion.cpp

#include <iostream>

// Recursive version: a function that calls itself.
// 5! = 5 * 4!  and 0! = 1 (the base case that stops the recursion).
unsigned long long factorialRecursive(int n) {
    if (n <= 1) {
        return 1;  // base case: stop calling ourselves
    }
    return static_cast<unsigned long long>(n) * factorialRecursive(n - 1);
}

// Iterative version: the same result using a loop (no recursion).
unsigned long long factorialIterative(int n) {
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= static_cast<unsigned long long>(i);
    }
    return result;
}

int main() {
    std::cout << "=== Factorial: recursion vs loop ===" << std::endl;
    std::cout << "n\trecursive\titerative" << std::endl;

    // Compare both versions for n = 0 .. 10.
    for (int n = 0; n <= 10; ++n) {
        unsigned long long rec = factorialRecursive(n);
        unsigned long long itr = factorialIterative(n);

        std::cout << n << "\t" << rec << "\t\t" << itr;

        // A small check so you can see both methods agree.
        if (rec == itr) {
            std::cout << "\t(same)";
        } else {
            std::cout << "\t(DIFFERENT!)";
        }
        std::cout << std::endl;
    }

    return 0;
}
