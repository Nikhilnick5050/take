// 04_bubble_sort.cpp
// Bubble sort on a std::vector<int>, with a helper that prints the vector.
// Compile: g++ -std=c++17 -Wall -o /tmp/x cpp/04_bubble_sort.cpp

#include <iostream>
#include <vector>    // for std::vector

// ---------------------------------------------------------------------------
// printVector: a small helper so we do not repeat the printing code.
// "const std::vector<int>&" means we only read the vector, we never change it.
// A vector knows its own size, so no separate size argument is needed.
// ---------------------------------------------------------------------------
void printVector(const std::vector<int>& numbers) {
    std::cout << "[ ";
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        std::cout << numbers[i] << " ";
    }
    std::cout << "]" << std::endl;
}

// ---------------------------------------------------------------------------
// BUBBLE SORT
// Idea: walk through the vector again and again. Whenever two neighbours are
// in the wrong order, swap them. After each pass the biggest remaining value
// "bubbles" to the end, so the vector gets sorted step by step.
// We pass the vector by reference (&) so the caller sees the sorted result.
// ---------------------------------------------------------------------------
void bubbleSort(std::vector<int>& numbers) {
    const std::size_t n = numbers.size();

    // We need at most n-1 passes over the vector.
    for (std::size_t pass = 0; pass + 1 < n; ++pass) {
        bool swapped = false;  // did anything change during this pass?

        // After "pass" passes, the last "pass" items are already in place,
        // so we can stop the inner loop earlier each time.
        for (std::size_t i = 0; i + 1 < n - pass; ++i) {
            if (numbers[i] > numbers[i + 1]) {
                // The two neighbours are in the wrong order -> swap them.
                int temp = numbers[i];
                numbers[i] = numbers[i + 1];
                numbers[i + 1] = temp;
                swapped = true;
            }
        }

        // If nothing was swapped, the vector is already sorted: stop early.
        // (Finishing on the very last pass is normal, so we only mention it
        //  when we really did stop before running out of passes.)
        if (!swapped) {
            if (pass + 2 < n) {
                std::cout << "  (stopped early after pass " << pass + 1
                          << " - the array was already sorted)" << std::endl;
            }
            break;
        }
    }
}

int main() {
    std::cout << "=== Bubble Sort ===" << std::endl;
    std::cout << std::endl;

    // A std::vector of ints. It knows its own size, so no size variable needed.
    std::vector<int> numbers = {42, 7, 19, 3, 88, 15, 3, 61, 0, 27};

    std::cout << "Before sorting:" << std::endl;
    printVector(numbers);
    std::cout << std::endl;

    bubbleSort(numbers);

    std::cout << "After sorting (smallest to largest):" << std::endl;
    printVector(numbers);
    std::cout << std::endl;

    // A second, tiny example so you can follow the swaps by eye.
    std::vector<int> small = {5, 1, 4, 2, 8};

    std::cout << "Second example:" << std::endl;
    std::cout << "  before: ";
    printVector(small);
    bubbleSort(small);
    std::cout << "  after:  ";
    printVector(small);

    return 0;
}
