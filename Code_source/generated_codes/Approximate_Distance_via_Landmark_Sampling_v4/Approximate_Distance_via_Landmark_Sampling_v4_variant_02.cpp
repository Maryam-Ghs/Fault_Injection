#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define NPOINTS 12           // small diverse dataset size
#define NLAND   4            // number of landmarks
#define PRINTOUT 3           // how many pair results to show

/* LLM input variant 2: small-diverse */

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

    // deterministic points (small diverse set)
    void generate_points() {
        static const float init_points[NPOINTS][2] = {
            {0.0f,   0.0f},
            {10.5f, 20.3f},
            {5.2f,  8.1f},
            {99.9f, 0.1f},
            {50.0f, 50.0f},
            {33.3f, 66.6f},
            {12.5f, 45.7f},
            {88.8f, 22.2f},
            {15.0f, 75.0f},
            {70.0f, 30.0f},
            {25.5f, 25.5f},
            {60.6f, 40.4f}
        };
        for (int i = 0; i < cnt; ++i) {
            px[i] = init_points[i][0];
            py[i] = init_points[i][1];
        }
    }

    // first lcnt points are used as landmarks, compute distances once
    void pick_landmarks_and_compute() {
        for (int i = 0; i < cnt; ++i) {
            for (int l = 0; l < lcnt; ++l) {
                float dx = px[i] - px[l];
                float dy = py[i] - py[l];
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
            float adiff = diff < 0.0f ? -diff : diff;
            best = adiff > best ? adiff : best;
        }
        return best;
    }

    // run a demo and print results
    void demo() {
        for (int i = 0; i < PRINTOUT; ++i) {
            int a = (i * 2) % cnt;
            int b = (i * 2 + 1) % cnt;
            float e = exact(a, b);
            float a_ = approx(a, b);
            printf("Pair %2d: exact=%.3f  approx=%.3f  error=%.3f\n",
                   i, e, a_, e - a_);
        }
    }
};

int main() {
    ApproxDist solver(NPOINTS, NLAND);
    solver.demo();
    return 0;
}
