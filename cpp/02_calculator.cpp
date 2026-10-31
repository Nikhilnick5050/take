// 02_calculator.cpp
// A small calculator built from functions + a switch statement.
// It uses FIXED sample inputs, so it never waits for keyboard input (no std::cin)
// and always finishes on its own.
// Compile: g++ -std=c++17 -Wall -o /tmp/x cpp/02_calculator.cpp

#include <iostream>  // std::cout (print output)

// One function per operation keeps main() short and easy to read.
double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

// Division needs a guard: dividing by zero is not allowed.
double divide(double a, double b) {
    if (b == 0.0) {
        std::cout << "  Error: cannot divide by zero!" << std::endl;
        return 0.0;  // we return 0 just so the program can continue
    }
    return a / b;
}

// The switch picks which function to call based on the operator character.
double calculate(double a, double b, char op) {
    switch (op) {
        case '+':
            return add(a, b);
        case '-':
            return subtract(a, b);
        case '*':
            return multiply(a, b);
        case '/':
            return divide(a, b);
        default:
            std::cout << "  Error: unknown operator '" << op << "'" << std::endl;
            return 0.0;
    }
}

int main() {
    std::cout << "=== Simple Calculator ===" << std::endl;
    std::cout << "Using fixed sample inputs (no keyboard input needed)." << std::endl;
    std::cout << std::endl;

    // Fixed sample inputs: first number, operator, second number.
    // The 5th sample divides by zero on purpose, to show the guard working.
    // The 6th uses an unknown operator, to show the switch default case.
    const double a[]  = {10.0, 10.0, 10.0, 10.0, 7.0, 9.0};
    const char   op[] = {'+',  '-',  '*',  '/',  '/',  '%'};
    const double b[]  = {5.0,  4.0,  3.0,  2.0,  0.0,  2.0};

    const int count = 6;  // how many samples we have

    // Go through every sample and print the expression and its answer.
    for (int i = 0; i < count; ++i) {
        std::cout << a[i] << " " << op[i] << " " << b[i] << std::endl;
        double result = calculate(a[i], b[i], op[i]);
        std::cout << "  = " << result << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Done." << std::endl;
    return 0;  // 0 means "the program finished without problems"
}
