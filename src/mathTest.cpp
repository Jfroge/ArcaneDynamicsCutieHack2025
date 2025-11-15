#include "../include/ArcaneMath.h"

int main() {
    // Initialize arrays so unspecified entries are deterministic
    float data[8] = {0.0f};
    bool known[8] = {false};
    data[0] = 9.8; // gravity = 9.8
    known [0] = true;
    data[7] = 10; // time = 10 seconds
    known [7] = true;
    data[5] = 50; // d = 50
    known[5] = true;
    data[6] = 45; // theta = 45
    known[6] = true;

    ArcaneMath test = ArcaneMath(data, known);
    test.print();
    test.solve();
    test.print();
}