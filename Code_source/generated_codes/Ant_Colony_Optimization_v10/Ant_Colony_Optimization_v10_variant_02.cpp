/* LLM input variant 2: small-diverse */
// Ant Colony Optimization - Version #10 (variant 2)
// Implements a simple ACO for a small TSP with varied ordinary values.
// Only int and float are used. All data structures are stack‑allocated.

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>    // for exp (float overload is fine)

#define MAX_N 8                 // maximum number of cities (stack size)
#define MAX_ANT 5               // number of ants per iteration (reduced)
#define MAX_ITER 15             // number of iterations (reduced)

// -------------------------------------------------------------------
// Helper: generate a distance matrix with varied ordinary values.
// Some distances are zero, some are moderate, and a few are large.
void build_graph(int cityCnt, int dist[MAX_N][MAX_N])
{
    // Initialise all distances to a large value (simulating no direct path)
    for (int i = 0; i < cityCnt; ++i) {
        for (int j = 0; j < cityCnt; ++j) {
            dist[i][j] = 9999;
        }
    }

    // Populate a set of diverse edges
    // Edge: (0,1) weight 12
    dist[0][1] = 12; dist[1][0] = 12;
    // Edge: (1,2) weight 0 (edge‑case)
    dist[1][2] = 0;  dist[2][1] = 0;
    // Edge: (2,3) weight 8
    dist[2][3] = 8;  dist[3][2] = 8;
    // Edge: (3,4) weight 20
    dist[3][4] = 20; dist[4][3] = 20;
    // Edge: (4,5) weight 5
    dist[4][5] = 5;  dist[5][4] = 5;
    // Edge: (5,0) weight 7
    dist[5][0] = 7;  dist[0][5] = 7;
    // Additional moderate edges to increase connectivity
    // Edge: (0,2) weight 14
    dist[0][2] = 14; dist[2][0] = 14;
    // Edge: (1,4) weight 22
    dist[1][4] = 22; dist[4][1] = 22;
    // Edge: (2,5) weight 13
    dist[2][5] = 13; dist[5][2] = 13;

    // Ensure diagonal is zero (distance to self)
    for (int i = 0; i < cityCnt; ++i) {
        dist[i][i] = 0;
    }
}

// -------------------------------------------------------------------
// Helper: initialise pheromone matrix with a small uniform value.
void init_pheromone(int cityCnt, float pher[MAX_N][MAX_N])
{
    for (int i = 0; i < cityCnt; ++i) {
        for (int j = 0; j < cityCnt; ++j) {
            pher[i][j] = 0.2f;   // slightly higher initial pheromone
        }
    }
}

// -------------------------------------------------------------------
// Helper: compute heuristic information (inverse of distance).
void compute_eta(int cityCnt, int dist[MAX_N][MAX_N], float eta[MAX_N][MAX_N])
{
    for (int i = 0; i < cityCnt; ++i) {
        for (int j = 0; j < cityCnt; ++j) {
            int d = dist[i][j];
            if (d == 0 && i != j) {
                eta[i][j] = 9999.0f;
            } else if (d == 0 && i == j) {
                eta[i][j] = 0.0f;
            } else {
                eta[i][j] = 1.0f / static_cast<float>(d);
            }
        }
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
    float alpha = 1.0f;   // influence of pheromone
    float beta  = 2.0f;   // influence of heuristic

    float numerators[MAX_N];
    for (int i = 0; i < cityCnt; ++i) {
        if (visited[i] || i == curCity) {
            numerators[i] = 0.0f;
        } else {
            float p1 = pher[curCity][i];
            float p2 = eta[curCity][i];
            float tmp1 = powf(p1, alpha);
            float tmp2 = powf(p2, beta);
            numerators[i] = tmp1 * tmp2;
        }
    }

    float denom = 0.0f;
    for (int i = 0; i < cityCnt; ++i) {
        denom += numerators[i];
    }

    for (int i = 0; i < cityCnt; ++i) {
        prob[i] = (denom == 0.0f) ? 0.0f : numerators[i] / denom;
    }
}

// -------------------------------------------------------------------
// Helper: select next city based on probabilities (roulette wheel).
int choose_next(int cityCnt, float prob[MAX_N])
{
    float cum[MAX_N];
    float sum = 0.0f;
    for (int i = 0; i < cityCnt; ++i) {
        sum += prob[i];
        cum[i] = sum;
    }

    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    int i = 0;
    while (i < cityCnt && cum[i] < r) ++i;
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
    bool visited[MAX_N] = {false};

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
    for (int i = 0; i < tourLen - 1; ++i) {
        total += dist[tour[i]][tour[i + 1]];
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
    for (int i = 0; i < cityCnt; ++i) {
        for (int j = 0; j < cityCnt; ++j) {
            pher[i][j] = (1.0f - evaporRate) * pher[i][j];
        }
    }

    for (int i = 0; i < bestLen - 1; ++i) {
        int a = bestTour[i];
        int b = bestTour[i + 1];
        pher[a][b] += deposit / static_cast<float>(bestLen);
        pher[b][a] += deposit / static_cast<float>(bestLen);
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

    // ------------------- Generate problem instance -------------------
    int N = 6;                         // reduced number of cities
    int distance[MAX_N][MAX_N];
    build_graph(N, distance);

    // ------------------- ACO parameters -------------------
    float pheromone[MAX_N][MAX_N];
    init_pheromone(N, pheromone);

    float heuristic[MAX_N][MAX_N];
    compute_eta(N, distance, heuristic);

    float evaporation = 0.25f;          // slightly lower evaporation
    float depositAmt   = 80.0f;         // reduced deposit amount

    // ------------------- Iterative optimization -------------------
    int globalBestTour[MAX_N];
    int globalBestLen = 999999;
    for (int iter = 0; iter < MAX_ITER; ++iter) {
        int iterBestTour[MAX_N];
        int iterBestLen = 999999;

        for (int antIdx = 0; antIdx < MAX_ANT; ++antIdx) {
            int curTour[MAX_N];
            int curLen = 0;
            build_tour(N, distance, pheromone, heuristic, curTour, curLen);
            int curDist = tour_length(N, distance, curTour, curLen);

            if (curDist < iterBestLen && curLen == N) {
                iterBestLen = curDist;
                for (int k = 0; k < N; ++k) {
                    iterBestTour[k] = curTour[k];
                }
            }
        }

        if (iterBestLen < globalBestLen) {
            globalBestLen = iterBestLen;
            for (int k = 0; k < N; ++k) {
                globalBestTour[k] = iterBestTour[k];
            }
        }

        if (iterBestLen < 999999) {
            pheromone_update(N, pheromone, iterBestTour, N,
                             evaporation, depositAmt);
        }
    }

    // ------------------- Output results -------------------
    std::cout << "Ant Colony Optimization (Version #10) Result\n";
    std::cout << "Best tour length : " << globalBestLen << "\n";
    std::cout << "Best tour order  : ";
    for (int i = 0; i < N; ++i) {
        std::cout << globalBestTour[i];
        if (i < N - 1) std::cout << " -> ";
    }
    std::cout << std::endl;
    return 0;
}
