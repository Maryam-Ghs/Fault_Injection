/* LLM input variant 6: ordered-structured */
// Ant Colony Optimization - Version 3
// ---------------------------------------------------------------
// This implementation solves a small TSP instance using ACO.
// Only int and float types are used, all data lives on the stack.
// ---------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define MAXC  12          // maximum number of cities (stack limit)
#define ANT_COUNT  30     // number of ants per iteration
#define ITER_MAX   100    // number of iterations
#define ALPHA      1.0f   // pheromone importance
#define BETA       5.0f   // heuristic importance
#define RHO        0.5f   // pheromone evaporation rate
#define QVAL      100.0f   // pheromone deposit factor

// ---------------------------------------------------------------
// Helper structures (all on stack)
// ---------------------------------------------------------------
struct City {
    int x;
    int y;
};

struct Ant {
    int path[MAXC];
    int visited[MAXC];
    float length;
};

// ---------------------------------------------------------------
// Generate an ordered, structured set of city coordinates (grid)
// ---------------------------------------------------------------
void generateCities(City city[], int n) {
    // Place cities on a regular grid with spacing 10
    int gridSize = (int)ceil(sqrt((double)n));
    int idx = 0;
    for (int row = 0; row < gridSize && idx < n; ++row) {
        for (int col = 0; col < gridSize && idx < n; ++col) {
            city[idx].x = col * 10 + 10;   // x = 10,20,30,...
            city[idx].y = row * 10 + 10;   // y = 10,20,30,...
            ++idx;
        }
    }
}

// ---------------------------------------------------------------
// Compute Euclidean distance matrix (float)
// ---------------------------------------------------------------
void computeDist(const City city[], float dist[][MAXC], int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int dx = city[i].x - city[j].x;
            int dy = city[i].y - city[j].y;
            float d = sqrtf((float)(dx * dx + dy * dy));
            dist[i][j] = d;
        }
    }
}

// ---------------------------------------------------------------
// Initialise pheromone matrix with uniform value
// ---------------------------------------------------------------
void initPheromone(float pher[][MAXC], int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            pher[i][j] = 1.0f;
        }
    }
}

// ---------------------------------------------------------------
// Construct a tour for a single ant using probabilistic choice
// ---------------------------------------------------------------
void constructTour(Ant &ant, const float pher[][MAXC],
                   const float dist[][MAXC], int n) {
    // reset visited flags
    for (int i = 0; i < n; ++i) ant.visited[i] = 0;

    // start from a random city
    int cur = rand() % n;
    ant.path[0] = cur;
    ant.visited[cur] = 1;

    // build the rest of the tour
    for (int step = 1; step < n; ++step) {
        // compute denominator of probability formula
        float denom = 0.0f;
        for (int cand = 0; cand < n; ++cand) {
            if (!ant.visited[cand]) {
                float tau = powf(pher[cur][cand], ALPHA);
                float eta = powf(1.0f / (dist[cur][cand] + 0.0001f), BETA);
                denom += tau * eta;
            }
        }

        // roulette wheel selection
        float pick = ((float)rand() / (float)RAND_MAX) * denom;
        float cumulative = 0.0f;
        int next = -1;
        for (int cand = 0; cand < n; ++cand) {
            if (!ant.visited[cand]) {
                float tau = powf(pher[cur][cand], ALPHA);
                float eta = powf(1.0f / (dist[cur][cand] + 0.0001f), BETA);
                cumulative += tau * eta;
                if (cumulative >= pick) {
                    next = cand;
                    break;
                }
            }
        }
        // fallback (should not happen)
        if (next == -1) {
            for (int cand = 0; cand < n; ++cand) {
                if (!ant.visited[cand]) {
                    next = cand;
                    break;
                }
            }
        }
        ant.path[step] = next;
        ant.visited[next] = 1;
        cur = next;
    }

    // compute tour length (including return to start)
    ant.length = 0.0f;
    for (int i = 0; i < n - 1; ++i) {
        ant.length += dist[ant.path[i]][ant.path[i + 1]];
    }
    ant.length += dist[ant.path[n - 1]][ant.path[0]];
}

// ---------------------------------------------------------------
// Update pheromone matrix based on all ants' tours
// ---------------------------------------------------------------
void evaporateAndDeposit(float pher[][MAXC], Ant ants[], int n) {
    // evaporation
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            pher[i][j] *= (1.0f - RHO);
        }
    }

    // deposit (each ant contributes)
    for (int a = 0; a < ANT_COUNT; ++a) {
        float contribution = QVAL / ants[a].length;
        for (int k = 0; k < n - 1; ++k) {
            int from = ants[a].path[k];
            int to   = ants[a].path[k + 1];
            pher[from][to] += contribution;
            pher[to][from] += contribution; // symmetric TSP
        }
        // add edge from last city back to first
        int last = ants[a].path[n - 1];
        int first = ants[a].path[0];
        pher[last][first] += contribution;
        pher[first][last] += contribution;
    }
}

// ---------------------------------------------------------------
// Main ACO driver
// ---------------------------------------------------------------
void runACO(int cityCount) {
    City city[MAXC];
    float dist[MAXC][MAXC];
    float pher[MAXC][MAXC];
    Ant ants[ANT_COUNT];

    generateCities(city, cityCount);
    computeDist(city, dist, cityCount);
    initPheromone(pher, cityCount);

    // keep best solution found
    float bestLen = 1e30f;
    int bestPath[MAXC];

    // main loop (iterative, loop‑heavy)
    for (int iter = 0; iter < ITER_MAX; ++iter) {
        // each ant builds a solution
        for (int a = 0; a < ANT_COUNT; ++a) {
            constructTour(ants[a], pher, dist, cityCount);
            if (ants[a].length < bestLen) {
                bestLen = ants[a].length;
                for (int i = 0; i < cityCount; ++i) {
                    bestPath[i] = ants[a].path[i];
                }
            }
        }
        // pheromone update
        evaporateAndDeposit(pher, ants, cityCount);
    }

    // output results
    printf("Best tour length: %.2f\n", bestLen);
    printf("Best tour order: ");
    for (int i = 0; i < cityCount; ++i) {
        printf("%d ", bestPath[i]);
    }
    printf("\n");
}

// ---------------------------------------------------------------
// Entry point (no external input)
// ---------------------------------------------------------------
int main() {
    srand((unsigned)time(nullptr));

    int nCities = 8;               // medium sized structured instance
    runACO(nCities);

    return 0;
}
