#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */
int main() {
    // Generate a relatively large but safe test set (1000 elements)
    std::vector<int> data;
    data.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        // Cycle through a safe range 0‑46340 to avoid overflow
        data.push_back(i % 46341);
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
