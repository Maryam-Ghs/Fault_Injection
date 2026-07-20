#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define N 200              // number of cities (large safe stress)
#define MAX_ITER 2000      // safety limit for iterations

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: compute Euclidean distance between two points
// ------------------------------------------------------------
float dist(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// ------------------------------------------------------------
// Helper: total length of a tour
// ------------------------------------------------------------
float tour_len(float xs[], float ys[], int order[])
{
    float sum = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        int a = order[i];
        int b = order[(i + 1) % N];
        sum += dist(xs[a], ys[a], xs[b], ys[b]);
    }
    return sum;
}

// ------------------------------------------------------------
// Helper: reverse segment [l .. r] in the tour (in‑place)
// ------------------------------------------------------------
void rev_seg(int order[], int l, int r)
{
    while (l < r)
    {
        int tmp = order[l];
        order[l] = order[r];
        order[r] = tmp;
        ++l;
        --r;
    }
}

// ------------------------------------------------------------
// 2‑Opt improvement step – returns true if an improvement was made
// ------------------------------------------------------------
bool improve_once(float xs[], float ys[], int order[])
{
    bool improved = false;

    // Outer loop – manual unrolling (process two i‑values per iteration)
    for (int i = 0; i < N - 2; i += 2)
    {
        // First i
        int i1 = order[i];
        int i2 = order[i + 1];
        float xi1 = xs[i1];
        float yi1 = ys[i1];
        float xi2 = xs[i2];
        float yi2 = ys[i2];
        float d_i = dist(xi1, yi1, xi2, yi2);

        // Second i (if within range)
        int i1b = -1, i2b = -1;
        float xi1b = 0.0f, yi1b = 0.0f, xi2b = 0.0f, yi2b = 0.0f, d_ib = 0.0f;
        if (i + 2 < N - 1)
        {
            i1b = order[i + 2];
            i2b = order[i + 3];
            xi1b = xs[i1b];
            yi1b = ys[i1b];
            xi2b = xs[i2b];
            yi2b = ys[i2b];
            d_ib = dist(xi1b, yi1b, xi2b, yi2b);
        }

        // Inner loop – also unrolled (process two j‑values per iteration)
        for (int j = i + 2; j < N - 1; j += 2)
        {
            // ---- pair (i , j) ----
            int a = order[i + 1];
            int b = order[j];
            int c = order[j + 1];
            float xa = xs[a];
            float ya = ys[a];
            float xb = xs[b];
            float yb = ys[b];
            float xc = xs[c];
            float yc = ys[c];

            float d1 = dist(xa, ya, xb, yb);
            float d2 = dist(xb, yb, xc, yc);
            float gain = (d_i + d2) - (d1 + d_i); // simplified gain = d2 - d1

            if (gain > 0.0001f)   // improvement found
            {
                rev_seg(order, i + 1, j);
                improved = true;
                return true;      // restart search after first improvement
            }

            // ---- pair (i , j+1) if j+1 < N-1 ----
            if (j + 2 < N)
            {
                int c2 = order[j + 2];
                float xc2 = xs[c2];
                float yc2 = ys[c2];
                float d3 = dist(xb, yb, xc2, yc2);
                float gain2 = (d_i + d3) - (d1 + d_i); // = d3 - d1

                if (gain2 > 0.0001f)
                {
                    rev_seg(order, i + 1, j + 1);
                    improved = true;
                    return true;
                }
            }

            // ---- second i (if existent) ----
            if (i1b != -1 && j + 2 < N)
            {
                int a2 = order[i + 3];
                int b2 = order[j];
                int c3 = order[j + 1];
                float xa2 = xs[a2];
                float ya2 = ys[a2];
                float xb2 = xs[b2];
                float yb2 = ys[b2];
                float xc3 = xs[c3];
                float yc3 = ys[c3];

                float d1b = dist(xa2, ya2, xb2, yb2);
                float d2b = dist(xb2, yb2, xc3, yc3);
                float gainb = (d_ib + d2b) - (d1b + d_ib); // = d2b - d1b

                if (gainb > 0.0001f)
                {
                    rev_seg(order, i + 3, j);
                    improved = true;
                    return true;
                }
            }
        }
    }
    return improved;
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    // Seed random generator
    srand((unsigned)time(0));

    // Stack‑allocated coordinate arrays
    float xs[N];
    float ys[N];

    // Generate large‑sized random points (range 0..10000)
    for (int k = 0; k < N; ++k)
    {
        xs[k] = (float)(rand() % 10001);
        ys[k] = (float)(rand() % 10001);
    }

    // Initial tour: 0,1,2,...,N-1
    int tour[N];
    for (int k = 0; k < N; ++k)
        tour[k] = k;

    // Initial length
    float best_len = tour_len(xs, ys, tour);

    // 2‑Opt main loop
    int iter = 0;
    while (iter < MAX_ITER)
    {
        bool changed = improve_once(xs, ys, tour);
        if (!changed) break;
        best_len = tour_len(xs, ys, tour);
        ++iter;
    }

    // Output results
    printf("Final tour length: %.3f\n", best_len);
    printf("Tour order: ");
    for (int i = 0; i < N; ++i)
        printf("%d ", tour[i]);
    printf("\n");
    return 0;
}
