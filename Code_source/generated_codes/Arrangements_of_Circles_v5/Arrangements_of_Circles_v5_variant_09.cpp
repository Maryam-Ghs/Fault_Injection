#include <iostream>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // Deterministic pseudo‑random looking input: 9 circles
    int count = 9;
    int* radii = new int[count];
    int init[9] = {7, 2, 9, 4, 1, 6, 3, 8, 5};
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
