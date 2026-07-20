#include <iostream>
#include <vector>

/* LLM input variant 6: ordered-structured */
int main() {
    // Sorted and highly structured input
    std::vector<int> data;
    for (int i = 0; i <= 6; ++i) {
        data.push_back(i);
    }

    std::size_t idx = 0;
    std::size_t total = data.size();

    while (idx + 1 < total) {
        int n1 = data[idx];
        int n2 = data[idx + 1];

        int prod1 = n1 * (n1 - 1);
        int res1  = prod1 + 2;
        std::cout << "n = " << n1 << " => regions = " << res1 << '\n';

        int prod2 = n2 * (n2 - 1);
        int res2  = prod2 + 2;
        std::cout << "n = " << n2 << " => regions = " << res2 << '\n';

        idx += 2;
    }

    if (idx < total) {
        int n = data[idx];
        int prod = n * (n - 1);
        int res  = prod + 2;
        std::cout << "n = " << n << " => regions = " << res << '\n';
    }

    return 0;
}
