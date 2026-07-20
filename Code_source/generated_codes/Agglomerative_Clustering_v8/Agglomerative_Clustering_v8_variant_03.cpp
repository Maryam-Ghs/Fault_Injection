/* LLM input variant 3: zeros-and-ones */
// Agglomerative Clustering – version #8
// Everything is inside main(), loop‑heavy, temporary variables, std::vector usage.

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // ---------- generate deterministic 2‑D points (zeros and ones) ----------
    int cntPts = 12;                     // number of data points
    int dim = 2;                         // dimensionality
    std::vector< std::vector<float> > pts(cntPts, std::vector<float>(dim));

    for (int i = 0; i < cntPts; ++i) {
        for (int j = 0; j < dim; ++j) {
            // Alternate between 0.0 and 1.0 based on point index
            pts[i][j] = (i % 2 == 0) ? 0.0f : 1.0f;
        }
    }

    // ---------- initial clusters (each point is its own cluster) ----------
    std::vector< std::vector<int> > clus(cntPts);
    for (int i = 0; i < cntPts; ++i) {
        clus[i].push_back(i);
    }

    // ---------- distance matrix (single‑link) ----------
    std::vector< std::vector<float> > dist(cntPts, std::vector<float>(cntPts, 0.0f));
    for (int a = 0; a < cntPts; ++a) {
        for (int b = a + 1; b < cntPts; ++b) {
            float sumSq = 0.0f;
            for (int d = 0; d < dim; ++d) {
                float diff = pts[a][d] - pts[b][d];
                float sq   = diff * diff;
                sumSq      += sq;
            }
            float eucl = std::sqrt(sumSq);
            dist[a][b] = eucl;
            dist[b][a] = eucl;
        }
    }

    // ---------- hierarchical merging ----------
    int targetClusters = 3;               // stop when we have this many clusters
    while ((int)clus.size() > targetClusters) {
        // find the pair of clusters with minimal distance
        float bestDist = 1e30f;
        int idxA = -1, idxB = -1;

        for (int p = 0; p < (int)clus.size(); ++p) {
            for (int q = p + 1; q < (int)clus.size(); ++q) {
                // single‑link distance: minimum distance between any two members
                float curMin = 1e30f;
                for (int u = 0; u < (int)clus[p].size(); ++u) {
                    for (int v = 0; v < (int)clus[q].size(); ++v) {
                        int idU = clus[p][u];
                        int idV = clus[q][v];
                        float dUV = dist[idU][idV];
                        if (dUV < curMin) curMin = dUV;
                    }
                }
                if (curMin < bestDist) {
                    bestDist = curMin;
                    idxA = p;
                    idxB = q;
                }
            }
        }

        // merge idxB into idxA (idxA < idxB guaranteed by loop order)
        for (int k = 0; k < (int)clus[idxB].size(); ++k) {
            clus[idxA].push_back(clus[idxB][k]);
        }
        // erase the merged cluster
        clus.erase(clus.begin() + idxB);
    }

    // ---------- output final clusters ----------
    std::cout << "Final clusters (version #8):\n";
    for (int c = 0; c < (int)clus.size(); ++c) {
        std::cout << "Cluster " << c << ": ";
        for (int t = 0; t < (int)clus[c].size(); ++t) {
            int pid = clus[c][t];
            std::cout << "(";
            for (int d = 0; d < dim; ++d) {
                std::cout << pts[pid][d];
                if (d + 1 < dim) std::cout << ", ";
            }
            std::cout << ")";
            if (t + 1 < (int)clus[c].size()) std::cout << ", ";
        }
        std::cout << "\n";
    }

    return 0;
}
