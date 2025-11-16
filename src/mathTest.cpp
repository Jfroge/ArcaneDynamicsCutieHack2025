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

    // Reset arrays for the second test
    for (int i = 0; i < 8; i++) {
        data[i] = 0.0f;
        known[i] = false;
    }
    
    data[6] = 45; // theta = 45
    known[6] = true;
    data[3] = 20;
    known[3] = true;

    test = ArcaneMath(data, known);

    test.print();
    test.solve();
    test.print();
    
    // Reset arrays for the second test
    for (int i = 0; i < 8; i++) {
        data[i] = 0.0f;
        known[i] = false;
    }
    
    data[6] = 45; // theta = 45
    known[6] = true;
    data[3] = 20; // vi
    known[3] = true;
    data[3] = 20; // yf
    known[3] = true;

    test = ArcaneMath(data, known);

    test.print();
    test.solve();
    test.print();

}