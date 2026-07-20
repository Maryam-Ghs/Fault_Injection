/* LLM input variant 9: medium-deterministic-random */
#include <cstdio>
#include <cstdlib>
#include <ctime>

// -------------------------------------------------------------------
// Helper: fill an integer array with random radii (1 … 20)
// -------------------------------------------------------------------
void generate_radii(int *rad, int count)
{
    std::srand(static_cast<unsigned>(std::time(0)));
    int idx = 0;
    while (idx < count)
    {
        int raw = std::rand();
        int limited = raw % 20;
        int radius = limited + 1;
        rad[idx] = radius;
        idx = idx + 1;
    }
}

// -------------------------------------------------------------------
// Helper: compute the total length of the line of touching circles
// -------------------------------------------------------------------
float compute_total_length(const int *rad, int n)
{
    if (n == 0) return 0.0f;
    float left_part = static_cast<float>(rad[0]) * 2.0f;
    float middle_sum = 0.0f;
    int i = 1;
    while (i < n)
    {
        int left_radius  = rad[i - 1];
        int right_radius = rad[i];
        int centre_gap   = left_radius + right_radius;
        middle_sum = middle_sum + static_cast<float>(centre_gap);
        i = i + 1;
    }
    float right_part = static_cast<float>(rad[n - 1]);
    float total = left_part + middle_sum - static_cast<float>(rad[0]) + right_part;
    return total;
}

// -------------------------------------------------------------------
// Helper: pretty‑print the array of radii (optional, for verification)
// -------------------------------------------------------------------
void show_radii(const int *rad, int n)
{
    std::printf("Radii (%d circles): ", n);
    for (int k = 0; k < n; ++k)
        std::printf("%d ", rad[k]);
    std::printf("\n");
}

// -------------------------------------------------------------------
// Main entry point
// -------------------------------------------------------------------
int main()
{
    const int MAX_CIRCLES = 30;
    const int circle_count = 30;

    int radii[MAX_CIRCLES];

    // Deterministic pseudo‑random sequence (LCG with fixed seed)
    const int preset_radii[30] = {
        12,5,9,14,3,17,8,2,19,6,
        11,4,15,1,13,7,16,10,18,20,
        9,3,14,5,12,2,17,8,6,11
    };
    for (int i = 0; i < circle_count; ++i)
        radii[i] = preset_radii[i];

    show_radii(radii, circle_count);

    float arrangement_length = compute_total_length(radii, circle_count);

    std::printf("Number of circles       : %d\n", circle_count);
    std::printf("Total length of line    : %.2f units\n", arrangement_length);
    return 0;
}
