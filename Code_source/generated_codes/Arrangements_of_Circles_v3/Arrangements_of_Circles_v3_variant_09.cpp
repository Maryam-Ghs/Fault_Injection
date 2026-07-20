#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */
int main() {
    // Deterministic pseudo‑random‑looking medium sized input (15 elements)
    std::vector<int> data;
    int preset[] = {12, 7, 19, 4, 23, 15, 8, 30, 11, 5, 18, 9, 27, 2, 14};
    for (int v : preset) {
        data.push_back(v);
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
