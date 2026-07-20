#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

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
// main – deterministic medium‑size pseudo‑randomish input
// ------------------------------------------------------------------
int main()
{
    // 12 circles with a deterministic pattern mimicking randomness
    int qty = 12;
    std::vector<float> radii = {
        0.3f, 1.4f, 2.5f, 3.6f, 4.7f,
        0.8f, 1.9f, 2.0f, 3.1f, 4.2f,
        5.0f, 0.4f
    };

    int ways = calc_fact(qty);
    float length = total_len(radii);

    std::cout << "Number of circles : " << qty << "\n";
    std::cout << "Radii              : ";
    show_vec(radii);
    std::cout << "Possible orders    : " << ways << "\n";
    std::cout << "Length of line (sum of diameters) : " << length << "\n";

    return 0;
}
