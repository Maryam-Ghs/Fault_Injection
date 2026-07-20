#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // ---------- problem size ----------
    const int nCities = 10;                 // number of nodes
    const int nAnts   = 20;                 // colony size
    const int nIter   = 100;                // generations

    // ---------- algorithm parameters ----------
    const int    alpha   = 1;               // pheromone importance
    const int    beta    = 5;               // heuristic importance
    const float  evap    = 0.5f;            // evaporation rate
    const float  Q       = 100.0f;          // pheromone deposit factor

    // ---------- deterministic reverse‑ordered distance matrix ----------
    int dist[20][20];
    for (int i = 0; i < nCities; ++i) {
        for (int j = 0; j < nCities; ++j) {
            if (i == j) {
                dist[i][j] = 0;
            } else if (i < j) {
                int d = 100 - (j - i) * 5;          // larger distance for close indices
                if (d < 10) d = 10;
                dist[i][j] = d;
                dist[j][i] = d;                    // symmetric
            }
        }
    }

    // ---------- heuristic matrix (1 / distance) ----------
    float heur[20][20];
    for (int i = 0; i < nCities; ++i) {
        for (int j = 0; j < nCities; ++j) {
            heur[i][j] = (i == j) ? 0.0f : 1.0f / static_cast<float>(dist[i][j]);
        }
    }

    // ---------- pheromone matrix ----------
    float pher[20][20];
    for (int i = 0; i < nCities; ++i) {
        for (int j = 0; j < nCities; ++j) {
            pher[i][j] = 1.0f;
        }
    }

    // ---------- best solution storage ----------
    float bestLen = 1e9f;
    int   bestTour[20] = {0};

    // ---------- main ACO loop ----------
    for (int iter = 0; iter < nIter; ++iter) {
        // each ant builds a tour
        for (int antIdx = 0; antIdx < nAnts; ++antIdx) {
            // deterministic start city: reverse order
            int start = nCities - 1 - (antIdx % nCities);
            int visited[20] = {0};
            int tour[20];
            int step = 0;
            int cur = start;
            visited[cur] = 1;
            tour[step++] = cur;

            // construct the remainder of the tour
            while (step < nCities) {
                // compute selection probabilities
                float prob[20] = {0.0f};
                float sum = 0.0f;
                for (int c = 0; c < nCities; ++c) {
                    if (!visited[c]) {
                        float val = powf(pher[cur][c], static_cast<float>(alpha)) *
                                    powf(heur[cur][c], static_cast<float>(beta));
                        prob[c] = val;
                        sum += val;
                    }
                }

                // deterministic "roulette": pick middle of cumulative range
                float pick = sum * 0.5f;
                float acc = 0.0f;
                int next = -1;
                for (int c = 0; c < nCities; ++c) {
                    acc += prob[c];
                    if (acc >= pick && prob[c] > 0.0f) {
                        next = c;
                        break;
                    }
                }
                // fallback (should not happen)
                if (next == -1) {
                    for (int c = 0; c < nCities; ++c) {
                        if (!visited[c]) { next = c; break; }
                    }
                }

                visited[next] = 1;
                tour[step++] = next;
                cur = next;
            }

            // compute length of this tour (return to start)
            float length = 0.0f;
            for (int k = 0; k < nCities - 1; ++k) {
                length += static_cast<float>(dist[tour[k]][tour[k + 1]]);
            }
            length += static_cast<float>(dist[tour[nCities - 1]][tour[0]]); // close loop

            // update best solution if improved
            if (length < bestLen) {
                bestLen = length;
                for (int t = 0; t < nCities; ++t) {
                    bestTour[t] = tour[t];
                }
            }

            // deposit pheromone of this ant
            float deposit = Q / length;
            for (int k = 0; k < nCities - 1; ++k) {
                int a = tour[k];
                int b = tour[k + 1];
                pher[a][b] += deposit;
                pher[b][a] += deposit;
            }
            // edge returning to start
            int a = tour[nCities - 1];
            int b = tour[0];
            pher[a][b] += deposit;
            pher[b][a] += deposit;
        }

        // evaporate pheromone globally
        for (int i = 0; i < nCities; ++i) {
            for (int j = 0; j < nCities; ++j) {
                pher[i][j] *= (1.0f - evap);
            }
        }
    }

    // ---------- output ----------
    std::cout << "Best tour length : " << bestLen << "\n";
    std::cout << "Best tour order  : ";
    for (int p = 0; p < nCities; ++p) {
        std::cout << bestTour[p] << (p + 1 == nCities ? "\n" : " -> ");
    }

    return 0;
}
