/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cmath>

class AggloCluster {
public:
    AggloCluster(int cnt) {
        nPts = cnt;
        // generate deterministic sparse, skewed test vectors
        xs = new float[nPts];
        ys = new float[nPts];
        for (int i = 0; i < nPts; ++i) {
            if (i < nPts - 2) {          // majority clustered at origin
                xs[i] = 0.0f;
                ys[i] = 0.0f;
            } else if (i == nPts - 2) { // first outlier far away
                xs[i] = 100.0f;
                ys[i] = 100.0f;
            } else {                     // second outlier in different direction
                xs[i] = 200.0f;
                ys[i] = 0.0f;
            }
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
                    float dx = xs[i] - xs[j];
                    float dy = ys[i] - ys[j];
                    float sq = dy * dy + dx * dx;
                    dmat[i * nPts + j] = sqrtf(sq);
                }
            }
        }
        // enforce single‑linkage: keep minimal distance between clusters
        for (int i = 0; i < nPts; ++i) {
            for (int j = i + 1; j < nPts; ++j) {
                if (lbl[i] != lbl[j]) {
                    float cur = dmat[i * nPts + j];
                    for (int p = 0; p < nPts; ++p) {
                        for (int q = 0; q < nPts; ++q) {
                            int cond = (lbl[p] == lbl[i] && lbl[q] == lbl[j]);
                            cur = (cond && dmat[p * nPts + q] < cur) ?
                                  dmat[p * nPts + q] : cur;
                        }
                    }
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
    // deterministic 10‑point sparse, skewed example
    AggloCluster ac(10);
    ac.run();
    ac.printResult();
    return 0;
}
