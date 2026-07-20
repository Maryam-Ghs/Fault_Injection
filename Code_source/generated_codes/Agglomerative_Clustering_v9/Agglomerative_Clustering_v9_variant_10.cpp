#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

class AggloCluster {
public:
    AggloCluster(int cnt) {
        nPts = cnt;
        // generate deterministic test vectors (grid for larger dataset)
        xs = new float[nPts];
        ys = new float[nPts];
        for (int i = 0; i < nPts; ++i) {
            xs[i] = static_cast<float>(i % 15) * 1.0f;          // x : 0,1,2,...,14,0,1,...
            ys[i] = static_cast<float>(i / 15) * 1.0f;          // y : 0 repeated 15 times, then 1, etc.
        }

        // initial cluster labels = point indices
        lbl = new int[nPts];
        sz  = new int[nPts];
        for (int i = 0; i < nPts; ++i) {
            lbl[i] = i;
            sz[i]  = 1;
        }

        // distance matrix on heap
        dmat = new float[nPts * nPts];
        recomputeDist();

        // storage for merge history
        mergeA = new int[nPts - 1];
        mergeB = new int[nPts - 1];
        mergeD = new float[nPts - 1];
    }

    ~AggloCluster() {
        delete[] xs;
        delete[] ys;
        delete[] lbl;
        delete[] sz;
        delete[] dmat;
        delete[] mergeA;
        delete[] mergeB;
        delete[] mergeD;
    }

    void run() {
        int steps = 0;
        while (steps < nPts - 1) {
            // find closest pair of distinct clusters
            float bestDist = 1e30f;
            int bestI = -1, bestJ = -1;
            for (int i = 0; i < nPts; ++i) {
                for (int j = i + 1; j < nPts; ++j) {
                    int li = lbl[i], lj = lbl[j];
                    // branch‑minimized test using ternary
                    bestDist = (li != lj && dmat[i * nPts + j] < bestDist) ?
                               dmat[i * nPts + j] : bestDist;
                    bestI = (li != lj && dmat[i * nPts + j] < bestDist) ?
                            i : bestI;
                    bestJ = (li != lj && dmat[i * nPts + j] < bestDist) ?
                            j : bestJ;
                }
            }

            // record the merge (using current labels)
            mergeA[steps] = lbl[bestI];
            mergeB[steps] = lbl[bestJ];
            mergeD[steps] = bestDist;

            // merge cluster B into A
            int target = lbl[bestI];
            int source = lbl[bestJ];
            for (int k = 0; k < nPts; ++k) {
                lbl[k] = (lbl[k] == source) ? target : lbl[k];
            }
            sz[target] += sz[source];
            sz[source] = 0;

            // recompute distances for new configuration
            recomputeDist();

            ++steps;
        }
    }

    void printResult() {
        std::cout << "Agglomerative clustering (version #9)\n";
        for (int i = 0; i < nPts - 1; ++i) {
            std::cout << "Step " << i + 1 << ": merge "
                      << mergeA[i] << " & " << mergeB[i]
                      << " at distance " << mergeD[i] << "\n";
        }
    }

private:
    int nPts;
    float* xs;
    float* ys;
    int* lbl;
    int* sz;
    float* dmat;
    int* mergeA;
    int* mergeB;
    float* mergeD;

    // recompute full distance matrix respecting current labels
    void recomputeDist() {
        for (int i = 0; i < nPts; ++i) {
            for (int j = 0; j < nPts; ++j) {
                if (i == j) {
                    dmat[i * nPts + j] = 0.0f;
                } else {
                    // compute Euclidean distance (order of operations reordered)
                    float dx = xs[i] - xs[j];
                    float dy = ys[i] - ys[j];
                    float sq = dy * dy + dx * dx;          // reordered sum
                    dmat[i * nPts + j] = sqrtf(sq);
                }
            }
        }
        // enforce single‑linkage: keep minimal distance between clusters
        for (int i = 0; i < nPts; ++i) {
            for (int j = i + 1; j < nPts; ++j) {
                if (lbl[i] != lbl[j]) {
                    float cur = dmat[i * nPts + j];
                    // scan all members of the two clusters to find the minimum
                    for (int p = 0; p < nPts; ++p) {
                        for (int q = 0; q < nPts; ++q) {
                            int cond = (lbl[p] == lbl[i] && lbl[q] == lbl[j]);
                            cur = (cond && dmat[p * nPts + q] < cur) ?
                                  dmat[p * nPts + q] : cur;
                        }
                    }
                    // assign the minimal distance to every cross‑pair
                    for (int p = 0; p < nPts; ++p) {
                        for (int q = 0; q < nPts; ++q) {
                            if (lbl[p] == lbl[i] && lbl[q] == lbl[j]) {
                                dmat[p * nPts + q] = cur;
                                dmat[q * nPts + p] = cur;
                            }
                        }
                    }
                }
            }
        }
    }
};

int main() {
    // deterministic 150‑point example for stress testing
    AggloCluster ac(150);
    ac.run();
    ac.printResult();
    return 0;
}
