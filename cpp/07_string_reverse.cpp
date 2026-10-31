// 07_string_reverse.cpp
// Reverses a std::string in two different ways:
//   1) by hand, with the "two-pointer" technique (a loop)
//   2) with std::reverse from the <algorithm> header, for comparison
//
// Compile: g++ -std=c++17 -Wall -o rev cpp/07_string_reverse.cpp
// Run:     ./rev
//          echo "hello world" | ./rev   (reverses your own text)

#include <iostream>
#include <string>
#include <algorithm>   // for std::reverse
#include <utility>     // for std::swap

// ---------------------------------------------------------------------------
// Version 1: the two-pointer technique.
//
// One pointer (left) starts at the first character, the other (right) starts
// at the last character. We swap those two characters, then move the pointers
// one step towards the middle. When they meet (or cross), the string is
// completely reversed.
//
// The string is passed by reference (std::string&) so we change the caller's
// string directly instead of working on a copy.
// ---------------------------------------------------------------------------
void reverseWithTwoPointers(std::string& s) {
    int left  = 0;                                  // index of first char
    int right = static_cast<int>(s.size()) - 1;     // index of last char

    while (left < right) {
        std::swap(s[left], s[right]);   // swap the two ends
        ++left;                         // move left pointer to the right
        --right;                        // move right pointer to the left
    }
    // The middle character of an odd-length string never moves, which is
    // exactly what we want.
}

// Version 2: build a NEW reversed string and leave the original untouched.
// Same idea, but we read the characters from the end towards the start.
std::string reversedCopy(const std::string& s) {
    std::string result;
    result.reserve(s.size());   // reserve space once, avoids re-allocations
    for (int i = static_cast<int>(s.size()) - 1; i >= 0; --i) {
        result.push_back(s[i]);
    }
    return result;
}

int main() {
    std::cout << "=== Reversing a string ===" << std::endl;

    // --- 1. The two-pointer technique on a few sample strings ------------
    std::string samples[] = {"Hello, C++17!", "level", "a", "", "12345"};
    const int sampleCount = 5;

    std::cout << "\n-- Two-pointer technique (reverses in place) --" << std::endl;
    for (int i = 0; i < sampleCount; ++i) {
        std::string original = samples[i];   // keep the "before" text
        std::string working  = samples[i];   // this copy gets reversed

        reverseWithTwoPointers(working);

        std::cout << "  \"" << original << "\"  ->  \"" << working << "\"";
        if (!original.empty() && original == working) {
            std::cout << "   (a palindrome!)";
        }
        std::cout << std::endl;
    }

    // --- 2. std::reverse does the same job in one line -------------------
    std::cout << "\n-- std::reverse from <algorithm> (for comparison) --"
              << std::endl;
    for (int i = 0; i < sampleCount; ++i) {
        std::string working = samples[i];
        std::reverse(working.begin(), working.end());   // the whole job
        std::cout << "  \"" << samples[i] << "\"  ->  \"" << working << "\""
                  << std::endl;
    }

    // --- 3. A version that returns a new string --------------------------
    std::cout << "\n-- Reversed copy (original stays unchanged) --" << std::endl;
    std::string text = "beginner friendly";
    std::cout << "  original: \"" << text << "\"" << std::endl;
    std::cout << "  reversed: \"" << reversedCopy(text) << "\"" << std::endl;

    // --- 4. Let the user try their own text ------------------------------
    std::cout << "\nType some text to reverse it (or press Enter to skip):"
              << std::endl;
    std::string userText;
    if (std::getline(std::cin, userText) && !userText.empty()) {
        std::string copy = userText;
        reverseWithTwoPointers(copy);
        std::cout << "  you typed: \"" << userText << "\"" << std::endl;
        std::cout << "  reversed : \"" << copy << "\"" << std::endl;
    } else {
        std::cout << "  (no text given, skipping)" << std::endl;
    }

    return 0;
}
