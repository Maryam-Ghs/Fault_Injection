#include <iostream>
#include <cstdlib>

/* LLM input variant 2: small-diverse */

int main() {
    // Small diverse input: 7 circles with varied radii
    int count = 7;
    int* radii = new int[count];
    int init[7] = {2, 5, 3, 8, 1, 6, 4};
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
