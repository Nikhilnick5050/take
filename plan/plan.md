# Plan — 10 C++ files with two helpers

Goal: create 10 standalone, compilable C++17 files in `cpp/` using two helper agents working in parallel.

## Steps
1. Helper 1 (can edit) → `cpp/01_hello_world.cpp` … `cpp/04_bubble_sort.cpp`
2. Helper 2 (can edit) → `cpp/05_fibonacci.cpp` … `cpp/08_matrix_multiplication.cpp`
3. Me (main agent) → `cpp/09_student_records.cpp`, `cpp/10_file_word_count.cpp`
4. I compile every file with `g++ -std=c++17` and run it to prove it works.
5. Report the results in chat.

## File set (beginner-friendly, one concept each)
| # | File | Concept |
|---|------|---------|
| 01 | hello_world.cpp | printing, main() |
| 02 | calculator.cpp | functions, switch, input |
| 03 | factorial_recursion.cpp | recursion |
| 04 | bubble_sort.cpp | arrays, sorting |
| 05 | fibonacci.cpp | loops, vectors |
| 06 | prime_check.cpp | loops, modulo |
| 07 | string_reverse.cpp | std::string, two pointers |
| 08 | matrix_multiplication.cpp | 2D arrays |
| 09 | student_records.cpp | structs, vectors |
| 10 | file_word_count.cpp | file I/O, fstream |

## Notes
- Each file: own `main()`, no shared headers, compiles alone.
- Helper 1 owns 01–04, Helper 2 owns 05–08, I own 09–10, so no file is edited twice.

## Result (done)
- All 10 files created in `cpp/`.
- Verified: `10/10 files compile cleanly` with `g++ -std=c++17 -Wall -Wextra`, and every program runs with exit code 0.
- Fixed after review: `04_bubble_sort.cpp` said "the vector was already sorted" while using a plain int array → now says "array".
- Helpers clashed twice on 02/04 and 05/06 (each re-applied its own spec version); final state verified by me, not by the helpers.
- Final verification by me: 10/10 compile with `g++ -std=c++17 -Wall` (0 errors, 0 warnings) and all 10 run with exit code 0, tested with empty stdin.
- Files 05, 06, 07 accept optional keyboard input but fall back to defaults when stdin is empty, so they never hang.
