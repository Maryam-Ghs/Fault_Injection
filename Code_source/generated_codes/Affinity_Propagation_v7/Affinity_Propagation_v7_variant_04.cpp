// LLM input variant 4: signed-extremes
// Affinity Propagation version #7
// ------------------------------------------------
// Implements the algorithm using only float and int,
// heap‑allocated arrays, manual loop unrolling and
// reordered arithmetic. No external input – data are
// generated internally.
// ------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>

class AffProp {
public:
    AffProp(int n, int iters, float damp)
        : nPoints(n), maxIter(iters), lambda(damp) {
        // allocate matrices on the heap
        simMat  = new float[nPoints * nPoints];
        rMat    = new float[nPoints * nPoints];
        aMat    = new float[nPoints * nPoints];
        // initialise to zero
        for (int i = 0; i < nPoints * nPoints; ++i) {
            rMat[i] = 0.0f;
            aMat[i] = 0.0f;
        }
        // fill similarity matrix with deterministic mixed values
        for (int i = 0; i < nPoints; ++i) {
            for (int j = 0; j < nPoints; ++j) {
                // produce a value in [-10,10] that includes negatives, zero, and positives
                // pattern: ((i - j) * 3) % 21 gives range [-20,20]; shift and scale
                int raw = ((i - j) * 3) % 21;
                if (raw < 0) raw += 21;            // ensure non‑negative modulo result
                float val = (static_cast<float>(raw) - 10.0f) * 0.5f; // now in [-5,5]
                simMat[i * nPoints + j] = val;
            }
        }
        // set preferences (diagonal) to median similarity later
        setPreferences();
    }

    ~AffProp() {
        delete[] simMat;
        delete[] rMat;
        delete[] aMat;
    }

    void run() {
        // main iteration loop (while version)
        int iter = 0;
        while (iter < maxIter) {
            updateResponsibility();
            updateAvailability();
            ++iter;
        }
        emitResult();
    }

private:
    int   nPoints;
    int   maxIter;
    float lambda;          // damping factor

    float* simMat;         // similarity matrix (n x n)
    float* rMat;           // responsibility matrix (n x n)
    float* aMat;           // availability matrix (n x n)

    // ------------------------------------------------
    // Helper to compute median and set diagonal (preference)
    // ------------------------------------------------
    void setPreferences() {
        // copy diagonal values to a temporary array
        float* diag = new float[nPoints];
        for (int i = 0; i < nPoints; ++i) {
            diag[i] = simMat[i * nPoints + i];
        }
        // simple selection sort to get median (n is small)
        for (int i = 0; i < nPoints - 1; ++i) {
            for (int j = i + 1; j < nPoints; ++j) {
                if (diag[j] < diag[i]) {
                    float tmp = diag[i];
                    diag[i] = diag[j];
                    diag[j] = tmp;
                }
            }
        }
        float median = diag[nPoints / 2];
        for (int i = 0; i < nPoints; ++i) {
            simMat[i * nPoints + i] = median;
        }
        delete[] diag;
    }

    // ------------------------------------------------
    // Responsibility update with manual loop unrolling
    // ------------------------------------------------
    void updateResponsibility() {
        for (int i = 0; i < nPoints; ++i) {
            // find two largest (value, index) of a(i,k)+s(i,k)
            float bestVal = -1e30f, secVal = -1e30f;
            int   bestIdx = -1;
            for (int k = 0; k < nPoints; k += 2) {
                // first element
                float cur0 = aMat[i * nPoints + k] + simMat[i * nPoints + k];
                if (cur0 > bestVal) {
                    secVal = bestVal;
                    bestVal = cur0;
                    bestIdx = k;
                } else if (cur0 > secVal) {
                    secVal = cur0;
                }
                // second element (if inside range)
                if (k + 1 < nPoints) {
                    float cur1 = aMat[i * nPoints + k + 1] + simMat[i * nPoints + k + 1];
                    if (cur1 > bestVal) {
                        secVal = bestVal;
                        bestVal = cur1;
                        bestIdx = k + 1;
                    } else if (cur1 > secVal) {
                        secVal = cur1;
                    }
                }
            }
            // update all responsibilities using the two maxima
            for (int k = 0; k < nPoints; k += 2) {
                // first k
                float oldR0 = rMat[i * nPoints + k];
                float maxEx0 = (k == bestIdx) ? secVal : bestVal;
                // reordered arithmetic: (1-λ)*new + λ*old
                float newR0 = (1.0f - lambda) * (simMat[i * nPoints + k] - maxEx0) + lambda * oldR0;
                rMat[i * nPoints + k] = newR0;
                // second k
                if (k + 1 < nPoints) {
                    float oldR1 = rMat[i * nPoints + k + 1];
                    float maxEx1 = ((k + 1) == bestIdx) ? secVal : bestVal;
                    float newR1 = (1.0f - lambda) * (simMat[i * nPoints + k + 1] - maxEx1) + lambda * oldR1;
                    rMat[i * nPoints + k + 1] = newR1;
                }
            }
        }
    }

    // ------------------------------------------------
    // Availability update with manual loop unrolling
    // ------------------------------------------------
    void updateAvailability() {
        for (int k = 0; k < nPoints; ++k) {
            // compute sum of positive responsibilities for column k (excluding i=k)
            float sumPos = 0.0f;
            for (int i = 0; i < nPoints; i += 2) {
                float r0 = rMat[i * nPoints + k];
                if (r0 > 0.0f) sumPos += r0;
                if (i + 1 < nPoints) {
                    float r1 = rMat[(i + 1) * nPoints + k];
                    if (r1 > 0.0f) sumPos += r1;
                }
            }
            // self‑availability a(k,k)
            float oldSelf = aMat[k * nPoints + k];
            float newSelf = (1.0f - lambda) * sumPos + lambda * oldSelf;
            aMat[k * nPoints + k] = newSelf;

            // off‑diagonal availabilities
            for (int i = 0; i < nPoints; i += 2) {
                if (i == k) {
                    // skip diagonal, already handled
                    if (i + 1 < nPoints && (i + 1) != k) {
                        float oldA = aMat[(i + 1) * nPoints + k];
                        float val = rMat[k * nPoints + k] + (sumPos - fmaxf(0.0f, rMat[(i + 1) * nPoints + k]));
                        if (val > 0.0f) val = 0.0f;
                        float newA = (1.0f - lambda) * val + lambda * oldA;
                        aMat[(i + 1) * nPoints + k] = newA;
                    }
                    continue;
                }
                // i != k
                float oldA = aMat[i * nPoints + k];
                float val = rMat[k * nPoints + k] + (sumPos - fmaxf(0.0f, rMat[i * nPoints + k]));
                if (val > 0.0f) val = 0.0f;
                float newA = (1.0f - lambda) * val + lambda * oldA;
                aMat[i * nPoints + k] = newA;
                // second element
                if (i + 1 < nPoints && (i + 1) != k) {
                    float oldA2 = aMat[(i + 1) * nPoints + k];
                    float val2 = rMat[k * nPoints + k] + (sumPos - fmaxf(0.0f, rMat[(i + 1) * nPoints + k]));
                    if (val2 > 0.0f) val2 = 0.0f;
                    float newA2 = (1.0f - lambda) * val2 + lambda * oldA2;
                    aMat[(i + 1) * nPoints + k] = newA2;
                }
            }
        }
    }

    // ------------------------------------------------
    // Emit exemplars and assignments
    // ------------------------------------------------
    void emitResult() {
        // Determine exemplars: max over k of a(i,k)+r(i,k)
        int* exemplar = new int[nPoints];
        for (int i = 0; i < nPoints; ++i) {
            float bestScore = -1e30f;
            int   bestIdx   = -1;
            for (int k = 0; k < nPoints; ++k) {
                float score = aMat[i * nPoints + k] + rMat[i * nPoints + k];
                if (score > bestScore) {
                    bestScore = score;
                    bestIdx   = k;
                }
            }
            exemplar[i] = bestIdx;
        }

        // Print results
        std::printf("Affinity Propagation Result (n=%d):\n", nPoints);
        std::printf("Point : Exemplar\n");
        for (int i = 0; i < nPoints; ++i) {
            std::printf("%5d : %8d\n", i, exemplar[i]);
        }
        delete[] exemplar;
    }
};

int main() {
    std::srand((unsigned)std::time(0));

    // deterministic small dataset with mixed signed values
    const int N      = 10;          // number of points
    const int ITER   = 50;          // max iterations
    const float DAMP = 0.5f;        // damping factor

    AffProp engine(N, ITER, DAMP);
    engine.run();

    return 0;
}
