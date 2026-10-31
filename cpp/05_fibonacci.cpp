// 05_fibonacci.cpp
// Prints the first N Fibonacci numbers using a simple loop and a std::vector.
//
// The Fibonacci sequence starts with 0 and 1, and every next number is the
// sum of the two numbers before it:
//     0, 1, 1, 2, 3, 5, 8, 13, 21, 34, ...
//
// N is read from the keyboard. If you just press Enter (or type something that
// is not a number), the program falls back to a default value, so it always
// finishes with useful output.
//
// Compile: g++ -std=c++17 -Wall -o fib cpp/05_fibonacci.cpp
// Run:     ./fib            (press Enter to use the default of 10)
//          echo 20 | ./fib  (ask for 20 numbers without typing)

#include <iostream>
#include <string>    // for std::string and std::getline
#include <vector>    // for std::vector

// Used when the user gives no usable number.
const int DEFAULT_COUNT = 10;

// The largest count we accept, so a typo like 1000000000 cannot flood the
// screen or keep the program busy for a very long time.
const int MAX_COUNT = 90;

// ---------------------------------------------------------------------------
// readCount(): asks the user how many numbers they want.
//
// We read a whole line with std::getline (so an empty line is easy to detect)
// and then try to turn that text into a number with std::stoi.
// std::stoi throws an exception when the text is not a number, so we catch it
// and quietly use the default instead. That is the "default fallback".
// ---------------------------------------------------------------------------
int readCount() {
    std::cout << "How many Fibonacci numbers do you want? (1-" << MAX_COUNT
              << ", press Enter for " << DEFAULT_COUNT << "): ";

    std::string line;
    if (!std::getline(std::cin, line)) {
        // No input at all (for example the input was piped and is empty).
        std::cout << "\n(no input, using the default of " << DEFAULT_COUNT
                  << ")" << std::endl;
        return DEFAULT_COUNT;
    }

    if (line.empty()) {
        std::cout << "(empty input, using the default of " << DEFAULT_COUNT
                  << ")" << std::endl;
        return DEFAULT_COUNT;
    }

    int count = DEFAULT_COUNT;
    try {
        count = std::stoi(line);   // may throw std::invalid_argument
    } catch (const std::exception&) {
        std::cout << "(\"" << line << "\" is not a number, using the default of "
                  << DEFAULT_COUNT << ")" << std::endl;
        return DEFAULT_COUNT;
    }

    // Keep the value inside a sensible range.
    if (count < 1) {
        std::cout << "(" << count << " is too small, using 1)" << std::endl;
        count = 1;
    } else if (count > MAX_COUNT) {
        std::cout << "(" << count << " is too big, using " << MAX_COUNT << ")"
                  << std::endl;
        count = MAX_COUNT;
    }

    return count;
}

// Builds and returns the first `count` Fibonacci numbers.
// We use std::vector<long long> so the numbers can grow quite large
// without overflowing a normal 32-bit int too early.
std::vector<long long> makeFibonacci(int count) {
    std::vector<long long> fib;   // empty vector, we fill it in the loop

    if (count <= 0) {
        return fib;               // nothing to do for 0 or negative counts
    }

    // The first two numbers are fixed by definition.
    fib.push_back(0);
    if (count == 1) {
        return fib;               // the user only wanted one number
    }
    fib.push_back(1);

    // Now compute the rest: each new number = sum of the last two.
    for (int i = 2; i < count; ++i) {
        long long next = fib[i - 1] + fib[i - 2];
        fib.push_back(next);
    }

    return fib;
}

int main() {
    std::cout << "=== Fibonacci numbers ===" << std::endl;

    // --- 1. Ask the user how many numbers to print (default fallback) ----
    const int count = readCount();

    // --- 2. Build the sequence and print it ------------------------------
    std::vector<long long> fib = makeFibonacci(count);

    std::cout << "\nThe first " << count << " Fibonacci numbers are:"
              << std::endl;
    for (int i = 0; i < static_cast<int>(fib.size()); ++i) {
        std::cout << "  F(" << i << ") = " << fib[i] << std::endl;
    }

    // A compact one-line version, handy for a quick look.
    std::cout << "\nIn one line: ";
    for (int i = 0; i < static_cast<int>(fib.size()); ++i) {
        std::cout << fib[i];
        if (i + 1 < static_cast<int>(fib.size())) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;

    return 0;
}
