#include <iostream>
#include <cstdlib>

/* LLM input variant 6: ordered-structured */

int main() {
    // Ordered, structured input: 8 circles with ascending radii
    int count = 8;
    int* radii = new int[count];
    int init[8] = {1, 2, 3, 4, 5, 6, 7, 8};
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
