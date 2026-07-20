#include <iostream>
#include <cstdlib>

/* LLM input variant 8: sparse-skewed */

int main() {
    // Sparse and skewed input: 10 circles with many zeros and a few non‑zeros
    int count = 10;
    int* radii = new int[count];
    int init[10] = {0, 0, 0, 9, 0, 0, 5, 0, 0, 1};
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
