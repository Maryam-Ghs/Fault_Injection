#include <iostream>
#include <vector>

/* LLM input variant 1: minimal-boundary */
int main() {
    // Minimal non‑trivial set of inputs
    std::vector<int> data;
    data.push_back(0);   // no circles
    data.push_back(1);   // single circle

    std::size_t idx = 0;
    std::size_t total = data.size();

    // Manual loop unrolling: handle two elements per iteration
    while (idx + 1 < total) {
        int n1 = data[idx];
        int n2 = data[idx + 1];

        // ----- first element -----
        int prod1 = n1 * (n1 - 1);
        int res1  = prod1 + 2;
        std::cout << "n = " << n1 << " => regions = " << res1 << '\n';

        // ----- second element -----
        int prod2 = n2 * (n2 - 1);
        int res2  = prod2 + 2;
        std::cout << "n = " << n2 << " => regions = " << res2 << '\n';

        idx += 2;
    }

    // If an odd element remains, process it alone
    if (idx < total) {
        int n = data[idx];
        int prod = n * (n - 1);
        int res  = prod + 2;
        std::cout << "n = " << n << " => regions = " << res << '\n';
    }

    return 0;
}
