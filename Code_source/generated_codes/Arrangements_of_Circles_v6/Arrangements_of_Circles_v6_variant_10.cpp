/* LLM input variant 10: large-safe-stress */
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
    int preview = (n < 20) ? n : 20;
    for (int k = 0; k < preview; ++k)
        std::printf("%d ", rad[k]);
    if (n > 20) std::printf("... (%d total)\n", n);
    else std::printf("\n");
}

// -------------------------------------------------------------------
// Main entry point
// -------------------------------------------------------------------
int main()
{
    const int MAX_CIRCLES = 5000;
    const int circle_count = 5000;

    int radii[MAX_CIRCLES];

    // Fill with a repeating pattern 1..20 to stay within bounds
    for (int i = 0; i < circle_count; ++i)
    {
        radii[i] = (i % 20) + 1;
    }

    show_radii(radii, circle_count);

    float arrangement_length = compute_total_length(radii, circle_count);

    std::printf("Number of circles       : %d\n", circle_count);
    std::printf("Total length of line    : %.2f units\n", arrangement_length);
    return 0;
}
