#include <iostream>
#include <cstdlib>

/* LLM input variant 10: large-safe-stress */

int main() {
    // Large but safe stress input: 12 circles (factorial fits in 32‑bit int)
    int count = 12;
    int* radii = new int[count];
    int init[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2};
    for (int i = 0; i < count; ++i) {
        radii[i] = init[i];
    }

    // ----- compute number of possible arrangements (factorial) -----
    int arrangements = 1;
    int multiplier = count;
    while (multiplier > 1) {
        arrangements = arrangements * multiplier;
        multiplier = multiplier - 1;
    }

    // ----- output the generated data and the result -----
    std::cout << "Number of circles : " << count << '\n';
    std::cout << "Radii : ";
    for (int i = 0; i < count; ++i) {
        std::cout << radii[i] << (i + 1 == count ? '\n' : ' ');
    }
    std::cout << "Possible arrangements (factorial) : " << arrangements << '\n';

    delete[] radii;
    return 0;
}
