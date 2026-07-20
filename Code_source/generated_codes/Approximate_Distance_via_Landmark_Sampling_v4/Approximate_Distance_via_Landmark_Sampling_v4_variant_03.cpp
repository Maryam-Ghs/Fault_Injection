/* LLM input variant 3: zeros-and-ones */
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define NPOINTS 2          // minimal size emphasizing zeros and ones
#define NLAND   2          // use both points as landmarks
#define PRINTOUT 1         // show a single pair result

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

    // deterministic points: alternating zeros and ones
    void generate_points() {
        for (int i = 0; i < cnt; ++i) {
            px[i] = (i % 2 == 0) ? 0.0f : 1.0f;
            py[i] = (i % 2 == 0) ? 0.0f : 1.0f;
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
    srand(0);                               // deterministic seed
    ApproxDist solver(NPOINTS, NLAND);
    solver.demo();                          // generate and show results
    return 0;
}
