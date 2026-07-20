// LLM input variant 1: minimal-boundary
// Ant Colony Optimization - Version #10
// Implements a simple ACO for a small TSP with edge‑case heavy input.
// Only int and float are used. All data structures are stack‑allocated.

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>    // for exp (float overload is fine)

#define MAX_N 2                 // minimum number of cities (stack size)
#define MAX_ANT 1               // one ant per iteration
#define MAX_ITER 1              // single iteration

// -------------------------------------------------------------------
// Helper: generate a distance matrix with minimal valid data.
// Uses only two cities with a single edge.
void build_graph(int cityCnt, int dist[MAX_N][MAX_N])
{
    int i = 0;
    while (i < cityCnt) {
        int j = 0;
        while (j < cityCnt) {
            dist[i][j] = 9999;          // default large distance
            ++j;
        }
        ++i;
    }

    // Edge between city 0 and city 1
    if (cityCnt >= 2) {
        dist[0][1] = 1;
        dist[1][0] = 1;
    }

    // Diagonal zero
    i = 0;
    while (i < cityCnt) {
        dist[i][i] = 0;
        ++i;
    }
}

// -------------------------------------------------------------------
// Helper: initialise pheromone matrix with a small uniform value.
void init_pheromone(int cityCnt, float pher[MAX_N][MAX_N])
{
    int i = 0;
    while (i < cityCnt) {
        int j = 0;
        while (j < cityCnt) {
            pher[i][j] = 0.1f;
            ++j;
        }
        ++i;
    }
}

// -------------------------------------------------------------------
// Helper: compute heuristic information (inverse of distance).
void compute_eta(int cityCnt, int dist[MAX_N][MAX_N], float eta[MAX_N][MAX_N])
{
    int i = 0;
    while (i < cityCnt) {
        int j = 0;
        while (j < cityCnt) {
            int d = dist[i][j];
            if (d == 0 && i != j) {
                eta[i][j] = 9999.0f;
            } else if (d == 0 && i == j) {
                eta[i][j] = 0.0f;
            } else {
                eta[i][j] = 1.0f / static_cast<float>(d);
            }
            ++j;
        }
        ++i;
    }
}

// -------------------------------------------------------------------
// Helper: compute transition probabilities for a given ant.
void calc_transition(int cityCnt,
                     int curCity,
                     bool visited[MAX_N],
                     float pher[MAX_N][MAX_N],
                     float eta[MAX_N][MAX_N],
                     float prob[MAX_N])
{
    float alpha = 1.0f;
    float beta  = 2.0f;

    float numerators[MAX_N];
    int i = 0;
    while (i < cityCnt) {
        if (visited[i] || i == curCity) {
            numerators[i] = 0.0f;
        } else {
            float p1 = pher[curCity][i];
            float p2 = eta[curCity][i];
            float tmp1 = powf(p1, alpha);
            float tmp2 = powf(p2, beta);
            numerators[i] = tmp1 * tmp2;
        }
        ++i;
    }

    float denom = 0.0f;
    i = 0;
    while (i < cityCnt) {
        denom += numerators[i];
        ++i;
    }

    i = 0;
    while (i < cityCnt) {
        if (denom == 0.0f) {
            prob[i] = 0.0f;
        } else {
            prob[i] = numerators[i] / denom;
        }
        ++i;
    }
}

// -------------------------------------------------------------------
// Helper: select next city based on probabilities (roulette wheel).
int choose_next(int cityCnt, float prob[MAX_N])
{
    float cum[MAX_N];
    float sum = 0.0f;
    int i = 0;
    while (i < cityCnt) {
        sum += prob[i];
        cum[i] = sum;
        ++i;
    }

    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    i = 0;
    while (i < cityCnt && cum[i] < r) {
        ++i;
    }
    if (i == cityCnt) i = cityCnt - 1;
    return i;
}

// -------------------------------------------------------------------
// Helper: construct a tour for a single ant.
void build_tour(int cityCnt,
                int dist[MAX_N][MAX_N],
                float pher[MAX_N][MAX_N],
                float eta[MAX_N][MAX_N],
                int tour[MAX_N],
                int &tourLen)
{
    bool visited[MAX_N];
    int i = 0;
    while (i < cityCnt) {
        visited[i] = false;
        ++i;
    }

    int start = rand() % cityCnt;
    tour[0] = start;
    visited[start] = true;
    int pos = 1;

    while (pos < cityCnt) {
        float prob[MAX_N];
        calc_transition(cityCnt, tour[pos - 1], visited, pher, eta, prob);
        int nxt = choose_next(cityCnt, prob);
        if (visited[nxt]) {
            int k = 0;
            while (k < cityCnt && visited[k]) ++k;
            if (k == cityCnt) break;
            nxt = k;
        }
        tour[pos] = nxt;
        visited[nxt] = true;
        ++pos;
    }

    tourLen = pos;
}

// -------------------------------------------------------------------
// Helper: evaluate length of a tour.
int tour_length(int cityCnt,
                int dist[MAX_N][MAX_N],
                int tour[MAX_N],
                int tourLen)
{
    int total = 0;
    int i = 0;
    while (i < tourLen - 1) {
        total += dist[tour[i]][tour[i + 1]];
        ++i;
    }
    if (tourLen == cityCnt) {
        total += dist[tour[tourLen - 1]][tour[0]];
    }
    return total;
}

// -------------------------------------------------------------------
// Helper: evaporate pheromone and deposit based on best tour.
void pheromone_update(int cityCnt,
                      float pher[MAX_N][MAX_N],
                      int bestTour[MAX_N],
                      int bestLen,
                      float evaporRate,
                      float deposit)
{
    int i = 0;
    while (i < cityCnt) {
        int j = 0;
        while (j < cityCnt) {
            pher[i][j] = (1.0f - evaporRate) * pher[i][j];
            ++j;
        }
        ++i;
    }

    i = 0;
    while (i < bestLen - 1) {
        int a = bestTour[i];
        int b = bestTour[i + 1];
        pher[a][b] += deposit / static_cast<float>(bestLen);
        pher[b][a] += deposit / static_cast<float>(bestLen);
        ++i;
    }
    if (bestLen == cityCnt) {
        int a = bestTour[bestLen - 1];
        int b = bestTour[0];
        pher[a][b] += deposit / static_cast<float>(bestLen);
        pher[b][a] += deposit / static_cast<float>(bestLen);
    }
}

// -------------------------------------------------------------------
// Main driver
int main()
{
    srand(static_cast<unsigned>(time(0)));

    int N = MAX_N;
    int distance[MAX_N][MAX_N];
    build_graph(N, distance);

    float pheromone[MAX_N][MAX_N];
    init_pheromone(N, pheromone);

    float heuristic[MAX_N][MAX_N];
    compute_eta(N, distance, heuristic);

    float evaporation = 0.3f;
    float depositAmt   = 100.0f;

    int globalBestTour[MAX_N];
    int globalBestLen = 999999;
    int iter = 0;
    while (iter < MAX_ITER) {
        int iterBestTour[MAX_N];
        int iterBestLen = 999999;

        int antIdx = 0;
        while (antIdx < MAX_ANT) {
            int curTour[MAX_N];
            int curLen = 0;
            build_tour(N, distance, pheromone, heuristic, curTour, curLen);
            int curDist = tour_length(N, distance, curTour, curLen);

            if (curDist < iterBestLen && curLen == N) {
                iterBestLen = curDist;
                int k = 0;
                while (k < N) {
                    iterBestTour[k] = curTour[k];
                    ++k;
                }
            }
            ++antIdx;
        }

        if (iterBestLen < globalBestLen) {
            globalBestLen = iterBestLen;
            int k = 0;
            while (k < N) {
                globalBestTour[k] = iterBestTour[k];
                ++k;
            }
        }

        if (iterBestLen < 999999) {
            pheromone_update(N, pheromone, iterBestTour, N,
                             evaporation, depositAmt);
        }

        ++iter;
    }

    std::cout << "Ant Colony Optimization (Version #10) Result\n";
    std::cout << "Best tour length : " << globalBestLen << "\n";
    std::cout << "Best tour order  : ";
    int i = 0;
    while (i < N) {
        std::cout << globalBestTour[i];
        if (i < N - 1) std::cout << " -> ";
        ++i;
    }
    std::cout << std::endl;
    return 0;
}
