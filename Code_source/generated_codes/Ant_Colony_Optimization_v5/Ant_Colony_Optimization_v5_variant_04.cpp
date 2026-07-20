#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 4: signed-extremes */

int main() {
    // ---------- problem size ----------
    int nCities = 10;                 // number of nodes
    int nAnts   = 20;                 // colony size
    int nIter   = 100;                // generations

    // ---------- algorithm parameters ----------
    int    alpha   = 1;               // pheromone importance
    int    beta    = 5;               // heuristic importance
    float  evap    = 0.5f;            // evaporation rate
    float  Q       = 100.0f;          // pheromone deposit factor

    // ---------- deterministic signed-extreme distance matrix ----------
    int dist[20][20];
    int i = 0;
    while (i < nCities) {
        int j = 0;
        while (j < nCities) {
            if (i == j) {
                dist[i][j] = 0;
            } else if (j > i) {
                // Mix negative, zero, and positive values: ((i+j)%5 - 2) * 20 yields -40, -20, 0, 20, 40
                int val = ((i + j) % 5 - 2) * 20;
                dist[i][j] = val;
                dist[j][i] = val; // symmetric
            }
            ++j;
        }
        ++i;
    }

    // ---------- heuristic matrix (1 / distance) ----------
    float heur[20][20];
    i = 0;
    while (i < nCities) {
        int j = 0;
        while (j < nCities) {
            if (i == j) {
                heur[i][j] = 0.0f;
            } else {
                heur[i][j] = 1.0f / (float)dist[i][j];
            }
            ++j;
        }
        ++i;
    }

    // ---------- pheromone matrix ----------
    float pher[20][20];
    i = 0;
    while (i < nCities) {
        int j = 0;
        while (j < nCities) {
            pher[i][j] = 1.0f;
            ++j;
        }
        ++i;
    }

    // ---------- best solution storage ----------
    float bestLen = 1e9f;
    int   bestTour[20];

    // ---------- main ACO loop ----------
    int iter = 0;
    while (iter < nIter) {
        // each ant builds a tour
        int antIdx = 0;
        while (antIdx < nAnts) {
            // start city is random
            int start = rand() % nCities;
            int visited[20] = {0};
            int tour[20];
            int step = 0;
            int cur = start;
            visited[cur] = 1;
            tour[step++] = cur;

            // construct the remainder of the tour
            while (step < nCities) {
                // compute selection probabilities
                float prob[20];
                float sum = 0.0f;
                int c = 0;
                while (c < nCities) {
                    if (!visited[c]) {
                        float val = powf(pher[cur][c], (float)alpha) *
                                    powf(heur[cur][c], (float)beta);
                        prob[c] = val;
                        sum += val;
                    } else {
                        prob[c] = 0.0f;
                    }
                    ++c;
                }

                // roulette wheel selection
                float pick = ((float)rand() / (float)RAND_MAX) * sum;
                float acc = 0.0f;
                int next = -1;
                c = 0;
                while (c < nCities) {
                    acc += prob[c];
                    if (acc >= pick) {
                        next = c;
                        break;
                    }
                    ++c;
                }
                // fallback (should not happen)
                if (next == -1) {
                    c = 0;
                    while (c < nCities) {
                        if (!visited[c]) { next = c; break; }
                        ++c;
                    }
                }

                visited[next] = 1;
                tour[step++] = next;
                cur = next;
            }

            // compute length of this tour (return to start)
            float length = 0.0f;
            int k = 0;
            while (k < nCities - 1) {
                length += (float)dist[tour[k]][tour[k+1]];
                ++k;
            }
            length += (float)dist[tour[nCities-1]][tour[0]]; // close loop

            // update best solution if improved
            if (length < bestLen) {
                bestLen = length;
                int t = 0;
                while (t < nCities) {
                    bestTour[t] = tour[t];
                    ++t;
                }
            }

            // deposit pheromone of this ant
            float deposit = Q / length;
            k = 0;
            while (k < nCities - 1) {
                int a = tour[k];
                int b = tour[k+1];
                pher[a][b] += deposit;
                pher[b][a] += deposit;
                ++k;
            }
            // edge returning to start
            int a = tour[nCities-1];
            int b = tour[0];
            pher[a][b] += deposit;
            pher[b][a] += deposit;

            ++antIdx;
        }

        // evaporate pheromone globally
        i = 0;
        while (i < nCities) {
            int j = 0;
            while (j < nCities) {
                pher[i][j] *= (1.0f - evap);
                ++j;
            }
            ++i;
        }

        ++iter;
    }

    // ---------- output ----------
    std::cout << "Best tour length : " << bestLen << "\n";
    std::cout << "Best tour order  : ";
    int p = 0;
    while (p < nCities) {
        std::cout << bestTour[p] << (p + 1 == nCities ? "\n" : " -> ");
        ++p;
    }

    return 0;
}
