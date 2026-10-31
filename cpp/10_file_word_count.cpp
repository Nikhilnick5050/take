// 10_file_word_count.cpp
// Reads a text file with std::ifstream and counts words, lines and characters.
// If the file does not exist yet, a small sample file is created first with
// std::ofstream, so this program always runs.
// Compile: g++ -std=c++17 -Wall -o /tmp/y cpp/10_file_word_count.cpp

#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main() {
    const std::string path = "sample.txt";

    // 1. Try to open the file for reading (ifstream = input file stream).
    std::ifstream in(path);

    // 2. If it is missing, create a small sample file first
    //    (ofstream = output file stream), then open it again.
    if (!in) {
        std::cout << path << " not found, creating a sample file...\n";
        {
            std::ofstream out(path);
            if (!out) {
                std::cerr << "Could not create " << path << "\n";
                return 1;
            }
            out << "The quick brown fox jumps over the lazy dog.\n"
                << "The dog sleeps and the fox runs away.\n"
                << "A quick fox is a happy fox.\n";
            out.close();   // closing flushes everything to disk
        }
        in.open(path);     // try again now that the file exists
        if (!in) {
            std::cerr << "Could not open " << path << " for reading\n";
            return 1;
        }
    }

    int lines = 0;
    int words = 0;
    int characters = 0;
    std::map<std::string, int> frequency;   // word -> how many times it appears

    std::string line;
    while (std::getline(in, line)) {
        ++lines;
        characters += static_cast<int>(line.size()) + 1;   // +1 for the newline

        std::istringstream stream(line);
        std::string word;
        while (stream >> word) {
            ++words;

            // Lowercase the word and strip punctuation so "Fox." matches "fox".
            std::string clean;
            for (char c : word) {
                if (std::isalpha(static_cast<unsigned char>(c))) {
                    clean += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            }
            if (!clean.empty()) {
                ++frequency[clean];
            }
        }
    }
    in.close();

    std::cout << "=== File statistics for " << path << " ===\n";
    std::cout << "Lines:      " << lines << "\n";
    std::cout << "Words:      " << words << "\n";
    std::cout << "Characters: " << characters << "\n\n";

    // 3. Show the three most common words.
    std::cout << "Most common words:\n";
    for (int i = 0; i < 3; ++i) {
        auto best = frequency.end();
        for (auto it = frequency.begin(); it != frequency.end(); ++it) {
            if (it->second > 0 && (best == frequency.end() || it->second > best->second)) {
                best = it;
            }
        }
        if (best == frequency.end()) break;
        std::cout << "  " << best->first << " -> " << best->second << " time(s)\n";
        best->second = 0;   // mark as used so it is not picked again
    }

    return 0;
}
