// 06_prime_check.cpp
// Two things in one small program:
//   1. check whether a number is prime, using a loop and the modulo operator
//   2. print every prime number from 2 up to 50
//
// A prime number is a whole number greater than 1 that can only be divided
// evenly (without a remainder) by 1 and by itself:  2, 3, 5, 7, 11, ...
//
// Compile: g++ -std=c++17 -Wall -o prime cpp/06_prime_check.cpp
// Run:     ./prime          (checks a few sample numbers)
//          echo 97 | ./prime (checks the number you type)

#include <iostream>
#include <limits>   // for std::numeric_limits

// ---------------------------------------------------------------------------
// isPrime(): returns true when n is a prime number, false otherwise.
//
// The idea: try to divide n by every number from 2 up to n/2.
// If any division has remainder 0 (n % d == 0), then d divides n evenly,
// so n is NOT prime. If we get through the whole loop, n is prime.
//
// Why stop at n/2? A divisor bigger than n/2 would need a partner smaller
// than 2, and the only such partner is 1 -- which every number has. So no
// divisor above n/2 can exist. (For n = 97 that is 48 checks instead of 96.)
// ---------------------------------------------------------------------------
bool isPrime(int n) {
    if (n < 2) {
        return false;   // 0, 1 and negative numbers are not prime
    }

    // Start at 2 and stop at n/2 (inclusive).
    for (int divisor = 2; divisor <= n / 2; ++divisor) {
        if (n % divisor == 0) {
            return false;   // found a divisor -> n is not prime
        }
    }

    return true;   // no divisor found -> n is prime
}

// Returns the smallest divisor of n (used only to explain the answer).
// Returns n itself when n is prime.
int smallestDivisor(int n) {
    for (int divisor = 2; divisor <= n / 2; ++divisor) {
        if (n % divisor == 0) {
            return divisor;
        }
    }
    return n;
}

// Prints a friendly line explaining whether n is prime.
void explain(int n) {
    if (isPrime(n)) {
        if (n < 4) {
            // 2 and 3 are so small that there is no divisor to test at all.
            std::cout << "  " << n << " is PRIME (nothing to divide it by)"
                      << std::endl;
        } else {
            std::cout << "  " << n << " is PRIME (no divisor from 2 up to "
                      << n / 2 << ")" << std::endl;
        }
    } else if (n < 2) {
        std::cout << "  " << n << " is NOT prime (primes start at 2)"
                  << std::endl;
    } else {
        int d = smallestDivisor(n);
        std::cout << "  " << n << " is NOT prime (" << d << " x "
                  << n / d << " = " << n << ")" << std::endl;
    }
}

int main() {
    std::cout << "=== Prime number check ===" << std::endl;

    // --- 1. Check a few sample numbers -----------------------------------
    std::cout << "\n-- Checking some sample numbers --" << std::endl;
    int samples[] = {1, 2, 3, 4, 17, 25, 29, 49, 97, 100};
    const int sampleCount = 10;
    for (int i = 0; i < sampleCount; ++i) {
        explain(samples[i]);
    }

    // --- 2. Print all primes up to 50 ------------------------------------
    std::cout << "\n-- All prime numbers up to 50 --" << std::endl;
    int count = 0;   // how many primes we found
    for (int n = 2; n <= 50; ++n) {
        if (isPrime(n)) {
            std::cout << n << " ";
            ++count;
        }
    }
    std::cout << "\n  (" << count << " primes found)" << std::endl;

    // --- 3. Let the user check their own number --------------------------
    std::cout << "\nType a number to check (or press Enter to skip): " << std::endl;
    int userNumber = 0;
    if (std::cin >> userNumber) {
        explain(userNumber);
    } else {
        std::cout << "  (no number given, skipping)" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return 0;
}
