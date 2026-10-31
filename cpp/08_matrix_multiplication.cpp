// 08_matrix_multiplication.cpp
// Multiplies two 3x3 matrices using 2D arrays and nested loops,
// then prints the result matrix.
//
// How matrix multiplication works:
//   C[i][j] = A[i][0]*B[0][j] + A[i][1]*B[1][j] + A[i][2]*B[2][j]
// In words: to get the value at row i, column j of the result, you walk along
// row i of A and down column j of B, multiplying the pairs and adding them up.
//
// Compile: g++ -std=c++17 -Wall -o matrix cpp/08_matrix_multiplication.cpp
// Run:     ./matrix

#include <iostream>
#include <iomanip>   // for std::setw (keeps the columns lined up)

const int N = 3;     // the matrices are 3 x 3

// Print a matrix as a neat table.
// The parameter is written as m[][N] because a 2D array needs its column
// count to be known when it is passed to a function.
void printMatrix(const int m[][N]) {
    for (int i = 0; i < N; ++i) {           // each row
        for (int j = 0; j < N; ++j) {       // each column in that row
            std::cout << std::setw(5) << m[i][j];   // width 5, right aligned
        }
        std::cout << std::endl;             // end of the row
    }
}

// Multiply two N x N matrices and store the answer in result.
void multiplyMatrices(const int a[][N], const int b[][N], int result[][N]) {
    // Start from all zeros: we add to result[i][j] inside the loop.
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = 0;
        }
    }

    // Three nested loops: row of A, column of B, and the shared index k.
    for (int i = 0; i < N; ++i) {           // row in A / row in result
        for (int j = 0; j < N; ++j) {       // column in B / column in result
            for (int k = 0; k < N; ++k) {   // walk along row i and column j
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

int main() {
    std::cout << "=== 3x3 Matrix multiplication ===" << std::endl;

    // --- The two input matrices ------------------------------------------
    int A[N][N] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    int B[N][N] = {
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    };

    int C[N][N];   // the result, filled in by multiplyMatrices()

    // --- Do the multiplication -------------------------------------------
    multiplyMatrices(A, B, C);

    // --- Show everything --------------------------------------------------
    std::cout << "\nMatrix A:" << std::endl;
    printMatrix(A);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(B);

    std::cout << "\nA x B =" << std::endl;
    printMatrix(C);

    // --- A second example: multiplying by the identity matrix -------------
    // The identity matrix I has 1s on the diagonal and 0s elsewhere.
    // A x I = A, which is a nice way to check that the code is correct.
    int I[N][N] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    int check[N][N];
    multiplyMatrices(A, I, check);

    std::cout << "\nA x I (identity) = A, so this should look like A:" << std::endl;
    printMatrix(check);

    return 0;
}
