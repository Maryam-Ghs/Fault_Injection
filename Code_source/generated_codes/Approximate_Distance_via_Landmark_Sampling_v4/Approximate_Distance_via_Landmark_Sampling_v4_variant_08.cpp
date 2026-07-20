#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define NPOINTS 120          // medium random array size
#define NLAND   15           // number of landmarks
#define PRINTOUT 5           // how many pair results to show

/* LLM input variant 8: sparse-skewed */

class ApproxDist {
    int   cnt;               // number of points
    int   lcnt;              // number of landmarks
    float* px;               // x‑coordinates (heap)
    float* py;               // y‑coordinates (heap)
    float* dl;               // distances to landmarks (heap), size cnt*lcnt

public:
    ApproxDist(int p, int l) : cnt(p), lcnt(l) {
        px = new float[cnt];
        py = new float[cnt];
        dl = new float[cnt * lcnt];
        generate_points();
        pick_landmarks_and_compute();
    }

    ~ApproxDist() {
        delete[] px;
        delete[] py;
        delete[] dl;
    }

    // generate skewed points: 80% clustered near (0,0), 20% scattered far away
    void generate_points() {
        int cluster_limit = static_cast<int>(cnt * 0.8);
        for (int i = 0; i < cnt; ++i) {
            if (i < cluster_limit) {
                // cluster near origin, range [0,10)
                px[i] = (float)(rand() % 1000) * 0.01f;   // 0‑9.99
                py[i] = (float)(rand() % 1000) * 0.01f;
            } else {
                // sparse outliers, range [80,100)
                px[i] = 80.0f + (float)(rand() % 2000) * 0.01f; // 80‑99.99
                py[i] = 80.0f + (float)(rand() % 2000) * 0.01f;
            }
        }
    }

    // first lcnt points are used as landmarks, compute distances once
    void pick_landmarks_and_compute() {
        for (int i = 0; i < cnt; ++i) {
            for (int l = 0; l < lcnt; ++l) {
                float dx = px[i] - px[l];
                float dy = py[i] - py[l];
                // reorder: compute squared sum then sqrt
                dl[i * lcnt + l] = sqrtf(dx * dx + dy * dy);
            }
        }
    }

    // exact Euclidean distance between two points (branch‑free math)
    float exact(int a, int b) {
        float dx = px[a] - px[b];
        float dy = py[a] - py[b];
        return sqrtf(dx * dx + dy * dy);
    }

    // landmark‑based approximation: max over |d(i,l)-d(j,l)|
    float approx(int a, int b) {
        float best = 0.0f;
        for (int l = 0; l < lcnt; ++l) {
            float diff = dl[a * lcnt + l] - dl[b * lcnt + l];
            float adiff = diff < 0.0f ? -diff : diff;   // branch‑minimized via ternary
            // reorder: compare then assign
            best = adiff > best ? adiff : best;
        }
        return best;
    }

    // run a demo and print results
    void demo() {
        for (int i = 0; i < PRINTOUT; ++i) {
            int a = rand() % cnt;
            int b = rand() % cnt;
            float e = exact(a, b);
            float a_ = approx(a, b);
            printf("Pair %2d: exact=%.3f  approx=%.3f  error=%.3f\n",
                   i, e, a_, e - a_);
        }
    }
};

int main() {
    srand(12345U);               // deterministic seed
    ApproxDist solver(NPOINTS, NLAND);
    solver.demo();                // generate and show results
    return 0;
}
