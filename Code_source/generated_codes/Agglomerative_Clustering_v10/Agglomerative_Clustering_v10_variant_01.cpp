/ * LLM input variant 1: minimal-boundary */
 // Agglomerative Clustering version #10
 #include <bits/stdc++.h>
 using namespace std;

 int main() {
     // ---------- generate minimal boundary data ----------
     // two duplicate points at the same location (zero distance)
     vector<float> xs = {0.0f, 0.0f};
     vector<float> ys = {0.0f, 0.0f};
     int n = xs.size();                     // number of points

     // ---------- initial clusters (each point alone) ----------
     vector<vector<int>> clusters;
     for (int i = 0; i < n; ++i) {
         clusters.push_back(vector<int>{i});
     }

     // ---------- distance matrix (upper triangle only) ----------
     vector<vector<float>> dmat(n, vector<float>(n, 0));
     for (int i = 0; i < n; ++i) {
         int j = i + 1;
         if (j < n) {
             float dx = xs[i] - xs[j];
             float dy = ys[i] - ys[j];
             dmat[i][j] = dx * dx + dy * dy;
         }
         ++j;
         if (j < n) {
             float dx = xs[i] - xs[j];
             float dy = ys[i] - ys[j];
             dmat[i][j] = dx * dx + dy * dy;
         }
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

         int csz = clusters.size();
         for (int a = 0; a < csz; ++a) {
             for (int b = a + 1; b < csz; ++b) {
                 float curMin = FLT_MAX;
                 for (int i = 0; i < (int)clusters[a].size(); ++i) {
                     int idxA = clusters[a][i];
                     int j = 0;
                     for (; j + 1 < (int)clusters[b].size(); j += 2) {
                         int idxB1 = clusters[b][j];
                         int idxB2 = clusters[b][j + 1];
                         float d1 = (idxA < idxB1) ? dmat[idxA][idxB1] : dmat[idxB1][idxA];
                         float d2 = (idxA < idxB2) ? dmat[idxA][idxB2] : dmat[idxB2][idxA];
                         curMin = (d1 < curMin) ? d1 : curMin;
                         curMin = (d2 < curMin) ? d2 : curMin;
                     }
                     for (; j < (int)clusters[b].size(); ++j) {
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
         clusters[bestA] = merged;
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
