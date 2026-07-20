// LLM input variant 8: sparse-skewed
// Agglomerative Clustering version #10
#include <bits/stdc++.h>
using namespace std;

int main() {
    // ---------- generate sparse‑skewed data ----------
    // a tight cluster of points near the origin and many isolated points far apart
    vector<float> xs = {
        0.0f, 0.1f, 0.2f, 0.15f, 0.05f,   // dense cluster
        10.0f, 20.0f, 30.0f, 40.0f, 50.0f,
        60.0f, 70.0f, 80.0f, 90.0f, 100.0f,
        110.0f, 120.0f, 130.0f, 140.0f, 150.0f   // isolated points
    };
    vector<float> ys = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // dense cluster
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f   // all on the x‑axis, far apart
    };
    int n = static_cast<int>(xs.size());                     // number of points

    // ---------- initial clusters (each point alone) ----------
    vector<vector<int>> clusters;
    clusters.reserve(n);
    for (int i = 0; i < n; ++i) {
        clusters.push_back(vector<int>{i});
    }

    // ---------- distance matrix (upper triangle only) ----------
    vector<vector<float>> dmat(n, vector<float>(n, 0));
    for (int i = 0; i < n; ++i) {
        // manual unrolling for the first two entries
        int j = i + 1;
        if (j < n) {
            float dx = xs[i] - xs[j];
            float dy = ys[i] - ys[j];
            dmat[i][j] = dx * dx + dy * dy;               // fused expression
        }
        ++j;
        if (j < n) {
            float dx = xs[i] - xs[j];
            float dy = ys[i] - ys[j];
            dmat[i][j] = dx * dx + dy * dy;
        }
        // remainder of the row
        for (j = i + 3; j < n; ++j) {
            float dx = xs[i] - xs[j];
            float dy = ys[i] - ys[j];
            dmat[i][j] = dx * dx + dy * dy;
        }
    }

    // ---------- agglomerative merging (single‑link) ----------
    int step = 0;
    while (clusters.size() > 1) {
        float bestDist = FLT_MAX;
        int bestA = -1, bestB = -1;

        int csz = static_cast<int>(clusters.size());
        // manual unrolling of the outer loop (pairwise cluster scan)
        for (int a = 0; a < csz; ++a) {
            for (int b = a + 1; b < csz; ++b) {
                // find the smallest inter‑cluster distance (single‑link)
                float curMin = FLT_MAX;

                // unroll member loops: two members of the second cluster at a time
                for (int i = 0; i < static_cast<int>(clusters[a].size()); ++i) {
                    int idxA = clusters[a][i];
                    int j = 0;
                    for (; j + 1 < static_cast<int>(clusters[b].size()); j += 2) {
                        int idxB1 = clusters[b][j];
                        int idxB2 = clusters[b][j + 1];
                        float d1 = (idxA < idxB1) ? dmat[idxA][idxB1] : dmat[idxB1][idxA];
                        float d2 = (idxA < idxB2) ? dmat[idxA][idxB2] : dmat[idxB2][idxA];
                        curMin = (d1 < curMin) ? d1 : curMin;
                        curMin = (d2 < curMin) ? d2 : curMin;
                    }
                    for (; j < static_cast<int>(clusters[b].size()); ++j) {
                        int idxB = clusters[b][j];
                        float d = (idxA < idxB) ? dmat[idxA][idxB] : dmat[idxB][idxA];
                        curMin = (d < curMin) ? d : curMin;
                    }
                }

                if (curMin < bestDist) {
                    bestDist = curMin;
                    bestA = a;
                    bestB = b;
                }
            }
        }

        // ---------- merge the selected pair ----------
        vector<int> merged = clusters[bestA];
        merged.insert(merged.end(), clusters[bestB].begin(), clusters[bestB].end());
        clusters[bestA] = std::move(merged);
        clusters.erase(clusters.begin() + bestB);

        // ---------- output current step ----------
        cout << "Step " << step++ << ": merge clusters " << bestA << " and " << bestB
             << " (distance " << sqrtf(bestDist) << ")\n";
    }

    // ---------- final result ----------
    cout << "Final cluster contains points: ";
    for (int id : clusters[0]) cout << id << ' ';
    cout << '\n';
    return 0;
}
