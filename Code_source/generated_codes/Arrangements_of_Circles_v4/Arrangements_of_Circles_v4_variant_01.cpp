#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------------
// Helper: create a random list of circle radii (float)
// ------------------------------------------------------------------
std::vector<float> build_radii(int cnt, float max_val)
{
    std::vector<float> out;
    out.reserve(cnt);
    int i = 0;
    while (i < cnt)
    {
        // generate a random float in (0, max_val]
        float r = static_cast<float>(std::rand() % 1000 + 1) / 1000.0f * max_val;
        out.push_back(r);
        ++i;
    }
    return out;
}

// ------------------------------------------------------------------
// Helper: compute factorial using only int arithmetic
// ------------------------------------------------------------------
int calc_fact(int n)
{
    int res = 1;
    for (int k = 2; k <= n; ++k)
        res = res * k;               // multiplication first, no extra ops
    return res;
}

// ------------------------------------------------------------------
// Helper: compute total length of a line‑wise arrangement
//          (sum of diameters) – order does not matter
// ------------------------------------------------------------------
float total_len(const std::vector<float>& rad)
{
    float sum = 0.0f;
    for (float v : rad)               // range‑based loop
        sum = sum + v;                // addition only
    // reorder: multiply after the sum is known
    return sum * 2.0f;                // diameter = 2 * radius
}

// ------------------------------------------------------------------
// Helper: pretty‑print a vector of floats
// ------------------------------------------------------------------
void show_vec(const std::vector<float>& vec)
{
    std::cout << "[ ";
    for (size_t p = 0; p < vec.size(); ++p)
    {
        std::cout << vec[p];
        if (p + 1 < vec.size())
            std::cout << ", ";
    }
    std::cout << " ]\n";
}

// ------------------------------------------------------------------
// main – deterministic minimal input
// ------------------------------------------------------------------
int main()
{
    // deterministic minimal input: 3 circles, each radius 0.1
    int qty = 3;
    std::vector<float> radii = {0.1f, 0.1f, 0.1f};

    int ways = calc_fact(qty);                    // N!
    float length = total_len(radii);

    std::cout << "Number of circles : " << qty << "\n";
    std::cout << "Radii              : ";
    show_vec(radii);
    std::cout << "Possible orders    : " << ways << "\n";
    std::cout << "Length of line (sum of diameters) : " << length << "\n";

    return 0;
}
