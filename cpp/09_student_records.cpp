// 09_student_records.cpp
// Concept: structs + std::vector — keeping a small list of records together.
// Compile: g++ -std=c++17 09_student_records.cpp -o student_records

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// A struct groups related values under one name.
struct Student {
    std::string name;
    int roll;       // roll number
    double marks;   // out of 100
};

// A small helper so we do not repeat the grade logic.
char gradeFor(double marks) {
    if (marks >= 90) return 'A';
    if (marks >= 75) return 'B';
    if (marks >= 60) return 'C';
    if (marks >= 40) return 'D';
    return 'F';
}

int main() {
    // A vector holds many Students, and can grow or shrink.
    std::vector<Student> students = {
        {"Riya Sharma", 12, 86.5},
        {"Aarav Patil", 7, 72.0},
        {"Meera Joshi", 21, 94.0},
        {"Kabir Deshmukh", 3, 58.5},
        {"Ananya Rao", 18, 39.0},
    };

    std::cout << "=== Student Records ===\n\n";
    std::cout << std::left << std::setw(18) << "Name"
              << std::setw(8) << "Roll"
              << std::setw(8) << "Marks"
              << "Grade\n";
    std::cout << std::string(42, '-') << "\n";

    double total = 0.0;
    for (const Student& s : students) {
        std::cout << std::left << std::setw(18) << s.name
                  << std::setw(8) << s.roll
                  << std::setw(8) << std::fixed << std::setprecision(1) << s.marks
                  << gradeFor(s.marks) << "\n";
        total += s.marks;
    }

    // Average marks of the class.
    const double average = students.empty() ? 0.0 : total / students.size();
    std::cout << std::string(42, '-') << "\n";
    std::cout << "Class average: " << std::fixed << std::setprecision(2)
              << average << "\n\n";

    // Sort a copy by marks, highest first, using a lambda.
    std::vector<Student> ranked = students;
    std::sort(ranked.begin(), ranked.end(),
              [](const Student& a, const Student& b) { return a.marks > b.marks; });

    std::cout << "Topper: " << ranked.front().name
              << " (" << std::fixed << std::setprecision(1) << ranked.front().marks << ")\n";

    std::cout << "\nRanking:\n";
    int rank = 1;
    for (const Student& s : ranked) {
        std::cout << "  " << rank++ << ". " << s.name << " - " << s.marks << "\n";
    }

    return 0;
}
