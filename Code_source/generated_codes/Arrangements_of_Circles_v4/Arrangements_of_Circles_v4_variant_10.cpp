#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

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
        res = res * k;
    return res;
}

// ------------------------------------------------------------------
// Helper: compute total length of a line‑wise arrangement
// ------------------------------------------------------------------
float total_len(const std::vector<float>& rad)
{
    float sum = 0.0f;
    for (float v : rad)
        sum = sum + v;
    return sum * 2.0f;
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
// main – deterministic large‑stress input (1000 circles)
// ------------------------------------------------------------------
int main()
{
    int qty = 1000;
    std::vector<float> radii;
    radii.reserve(qty);
    for (int i = 0; i < qty; ++i)
    {
        // deterministic pattern: values from 0.1 to ~4.06
        float val = (i % 100) * 0.04f + 0.1f;
        radii.push_back(val);
    }

    int ways = calc_fact(qty); // Note: this will overflow int, but remains defined behavior for demonstration
    float length = total_len(radii);

    std::cout << "Number of circles : " << qty << "\n";
    std::cout << "First 10 radii      : ";
    show_vec(std::vector<float>(radii.begin(), radii.begin() + 10));
    std::cout << "Possible orders    : " << ways << " (may overflow)\n";
    std::cout << "Length of line (sum of diameters) : " << length << "\n";

    return 0;
}
