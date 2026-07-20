/* LLM input variant 4: signed-extremes */
// Approximate Distance via Landmark Sampling - version #8
// -----------------------------------------------------
// Generates a random set of points, selects landmarks, pre‑computes
// distances to landmarks and answers a few approximate distance queries.
// Manual loop unrolling, fused arithmetic, stack allocation and helper functions
// are used throughout.  Only int and float types are employed.

#include <cstdio>
#include <cstdlib>
#include <cmath>

#define N 512          // number of points (must fit on stack)
#define L 16           // number of landmarks (multiple of 4 for unrolling)
#define Q 8            // number of queries to answer

// ---------------------------------------------------------------------
// Fill the coordinate arrays with values spanning negative, zero, and positive ranges.
void genPoints(int cnt, float xs[], float ys[])
{
    for (int i = 0; i < cnt; ++i)
    {
        // Base coordinate in [-500, 500)
        float base = (float)(rand() % 100000) / 100.0f - 500.0f;
        xs[i] = base;

        // Introduce zeros periodically and otherwise similar range for y
        if (i % 3 == 0)
            ys[i] = 0.0f;                         // explicit zero
        else
            ys[i] = (float)(rand() % 100000) / 100.0f - 500.0f;
    }
}

// ---------------------------------------------------------------------
// Pick L distinct landmark indices (simple deterministic selection)
void pickLandmarks(int cnt, int lm[])
{
    int step = cnt / L;
    for (int i = 0; i < L; ++i)
        lm[i] = i * step;
}

// ---------------------------------------------------------------------
// Compute Euclidean distance between two points (fused expression)
inline float pointDist(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx * dx + dy * dy);
}

// ---------------------------------------------------------------------
// Pre‑compute distances from every point to every landmark
// Manual unrolling by 4 landmarks per iteration.
void precompDist(int cnt, int lm[], float xs[], float ys[], float dmat[][L])
{
    for (int i = 0; i < cnt; ++i)
    {
        float xi = xs[i];
        float yi = ys[i];

        int j = 0;
        for (; j + 3 < L; j += 4)
        {
            int l0 = lm[j];
            int l1 = lm[j + 1];
            int l2 = lm[j + 2];
            int l3 = lm[j + 3];

            // fused computation of 4 distances
            float d0 = pointDist(xi, yi, xs[l0], ys[l0]);
            float d1 = pointDist(xi, yi, xs[l1], ys[l1]);
            float d2 = pointDist(xi, yi, xs[l2], ys[l2]);
            float d3 = pointDist(xi, yi, xs[l3], ys[l3]);

            dmat[i][j]     = d0;
            dmat[i][j + 1] = d1;
            dmat[i][j + 2] = d2;
            dmat[i][j + 3] = d3;
        }
        // remainder (should be none for L=16)
        for (; j < L; ++j)
        {
            int ll = lm[j];
            dmat[i][j] = pointDist(xi, yi, xs[ll], ys[ll]);
        }
    }
}

// ---------------------------------------------------------------------
// Approximate distance between two points a and b using landmarks
// Approximation: max |dist(a,ℓ) - dist(b,ℓ)|
float approxDist(int a, int b, float dmat[][L])
{
    float best = 0.0f;
    int j = 0;
    for (; j + 3 < L; j += 4)
    {
        float diff0 = dmat[a][j]     - dmat[b][j];
        float diff1 = dmat[a][j + 1] - dmat[b][j + 1];
        float diff2 = dmat[a][j + 2] - dmat[b][j + 2];
        float diff3 = dmat[a][j + 3] - dmat[b][j + 3];

        // absolute values fused with max updates
        float cand0 = diff0 < 0.0f ? -diff0 : diff0;
        float cand1 = diff1 < 0.0f ? -diff1 : diff1;
        float cand2 = diff2 < 0.0f ? -diff2 : diff2;
        float cand3 = diff3 < 0.0f ? -diff3 : diff3;

        best = cand0 > best ? cand0 : best;
        best = cand1 > best ? cand1 : best;
        best = cand2 > best ? cand2 : best;
        best = cand3 > best ? cand3 : best;
    }
    for (; j < L; ++j)
    {
        float diff = dmat[a][j] - dmat[b][j];
        float cand = diff < 0.0f ? -diff : diff;
        best = cand > best ? cand : best;
    }
    return best;
}

// ---------------------------------------------------------------------
// Main driver: generate data, run pre‑computation, answer queries.
int main()
{
    // seed random generator
    srand(12345);

    // stack‑allocated storage
    float xs[N];
    float ys[N];
    int   lm[L];
    float d2l[N][L];   // distances to landmarks

    // 1) generate random points with signed extremes
    genPoints(N, xs, ys);

    // 2) pick landmarks
    pickLandmarks(N, lm);

    // 3) pre‑compute distances to landmarks
    precompDist(N, lm, xs, ys, d2l);

    // 4) answer a few random approximate distance queries
    for (int q = 0; q < Q; ++q)
    {
        int u = rand() % N;
        int v = rand() % N;
        float approx = approxDist(u, v, d2l);
        printf("Query %d: approxDist(%d,%d) = %.3f\n", q, u, v, approx);
    }

    return 0;
}
