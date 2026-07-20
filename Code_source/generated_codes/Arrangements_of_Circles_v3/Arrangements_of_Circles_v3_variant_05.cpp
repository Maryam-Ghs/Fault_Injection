#include <iostream>
#include <vector>

/* LLM input variant 5: duplicate-heavy */
int main() {
    // Repeated values and ties
    std::vector<int> data;
    data.push_back(3);
    data.push_back(3);
    data.push_back(3);
    data.push_back(2);
    data.push_back(2);
    data.push_back(2);
    data.push_back(1);
    data.push_back(1);
    data.push_back(0);
    data.push_back(0);

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
