#include <iostream>
#include <cstdlib>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Duplicate-heavy input: 8 circles, all radii identical (3)
    int count = 8;
    int* radii = new int[count];
    for (int i = 0; i < count; ++i) {
        radii[i] = 3;
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
