// 01_hello_world.cpp
// Your very first C++ program.
// Compile: g++ -std=c++17 -Wall -o /tmp/x cpp/01_hello_world.cpp

#include <iostream>  // gives us std::cout (printing to the screen)

int main() {
    // std::cout prints text; std::endl moves to a new line.
    std::cout << "Hello, World!" << std::endl;

    // A few more lines so you can see how printing works.
    std::cout << "This is my first C++ program." << std::endl;
    std::cout << "C++17 is the language standard we are using." << std::endl;

    // You can print numbers and text together.
    std::cout << "2 + 3 = " << 2 + 3 << std::endl;

    // main() returns 0 to tell the operating system "all good".
    return 0;
}
