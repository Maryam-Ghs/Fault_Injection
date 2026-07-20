/* LLM input variant 1: minimal-boundary */
#include <cstdio>
#include <cstdlib>
#include <ctime>

// -------------------------------------------------------------------
// Helper: fill an integer array with random radii (1 … 20)
// -------------------------------------------------------------------
void generate_radii(int *rad, int count)
{
    // seed once – using time as source
    std::srand(static_cast<unsigned>(std::time(0)));

    // fill step‑by‑step, verbose naming
    int idx = 0;
    while (idx < count)
    {
        int raw = std::rand();                 // raw random integer
        int limited = raw % 20;                // limit to 0‑19
        int radius = limited + 1;              // shift to 1‑20
        rad[idx] = radius;                     // store
        idx = idx + 1;                         // advance
    }
}

// -------------------------------------------------------------------
// Helper: compute the total length of the line of touching circles
// -------------------------------------------------------------------
float compute_total_length(const int *rad, int n)
{
    // Edge case: no circles
    if (n == 0)
        return 0.0f;

    // Step 1 – length contributed by the leftmost circle (its left diameter)
    float left_part = static_cast<float>(rad[0]) * 2.0f;

    // Step 2 – accumulate distances between successive circle centres
    float middle_sum = 0.0f;
    int i = 1;
    while (i < n)
    {
        int left_radius  = rad[i - 1];
        int right_radius = rad[i];
        int centre_gap   = left_radius + right_radius;   // distance between centres
        middle_sum = middle_sum + static_cast<float>(centre_gap);
        i = i + 1;
    }

    // Step 3 – the rightmost circle contributes its right half (its radius)
    float right_part = static_cast<float>(rad[n - 1]);

    // Combine the three pieces, reordering additions (mathematically valid)
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
    {
        std::printf("%d ", rad[k]);
    }
    std::printf("\n");
}

// -------------------------------------------------------------------
// Main entry point – no external input, everything generated internally
// -------------------------------------------------------------------
int main()
{
    // ----------------------------------------------------------------
    // 1. Define problem size – minimal non‑trivial input
    // ----------------------------------------------------------------
    const int MAX_CIRCLES = 1;
    const int circle_count = 1;

    // ----------------------------------------------------------------
    // 2. Allocate stack array for radii
    // ----------------------------------------------------------------
    int radii[MAX_CIRCLES];

    // ----------------------------------------------------------------
    // 3. Deterministic input data
    // ----------------------------------------------------------------
    const int preset_radii[1] = {1};   // smallest valid radius
    for (int i = 0; i < circle_count; ++i)
        radii[i] = preset_radii[i];

    // ----------------------------------------------------------------
    // 4. (Optional) display a short preview of the generated data
    // ----------------------------------------------------------------
    show_radii(radii, circle_count);

    // ----------------------------------------------------------------
    // 5. Compute the total length of the arrangement
    // ----------------------------------------------------------------
    float arrangement_length = compute_total_length(radii, circle_count);

    // ----------------------------------------------------------------
    // 6. Output the result
    // ----------------------------------------------------------------
    std::printf("Number of circles       : %d\n", circle_count);
    std::printf("Total length of line    : %.2f units\n", arrangement_length);
    return 0;
}
