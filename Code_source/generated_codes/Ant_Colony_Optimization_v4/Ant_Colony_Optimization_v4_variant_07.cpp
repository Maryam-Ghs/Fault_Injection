/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define N 5               // number of cities
#define ANT_NUM 10        // number of ants
#define ITER_MAX 20       // number of iterations
#define ALPHA 1.0f        // importance of pheromone
#define BETA 5.0f         // importance of heuristic
#define RHO 0.5f          // evaporation rate
#define QVAL 100.0f       // pheromone deposit factor

/* ---------- memory helpers ---------- */
void makeMatrix(float ***mat) {
    int i = 0;
    while (i < N) {
        (*mat)[i] = new float[N];
        i = i + 1;
    }
}

/* ---------- problem data ---------- */
void loadDist(float **dist) {
    /* adversarial reverse-ordered symmetric distance matrix */
    int val = 50;  // start with a relatively large distance
    int i = 0;
    while (i < N) {
        int j = 0;
        while (j < N) {
            if (i == j) {
                dist[i][j] = 0.0f;
            } else if (j > i) {
                dist[i][j] = static_cast<float>(val);
                dist[j][i] = static_cast<float>(val);
                if (val > 1) val = val - 1;  // decrement to create descending order
            }
            j = j + 1;
        }
        i = i + 1;
    }
}

/* ---------- pheromone & heuristic ---------- */
void initPher(float **pher) {
    int i = 0;
    while (i < N) {
        int j = 0;
        while (j < N) {
            pher[i][j] = 1.0f;            // uniform start
            j = j + 1;
        }
        i = i + 1;
    }
}

/* heuristic = 1 / distance (avoid division by zero) */
void initHeur(float **heur, float **dist) {
    int i = 0;
    while (i < N) {
        int j = 0;
        while (j < N) {
            if (dist[i][j] == 0.0f) {
                heur[i][j] = 0.0f;
            } else {
                heur[i][j] = 1.0f / dist[i][j];
            }
            j = j + 1;
        }
        i = i + 1;
    }
}

/* ---------- solution construction ---------- */
void buildTour(int *tour, float **pher, float **heur, int startCity) {
    /* visited flag array */
    int *seen = new int[N];
    int k = 0;
    while (k < N) {
        seen[k] = 0;
        k = k + 1;
    }

    /* first city is fixed */
    tour[0] = startCity;
    seen[startCity] = 1;

    int pos = 1;
    while (pos < N) {
        int cur = tour[pos - 1];
        /* compute selection probabilities for remaining cities */
        float prob[N];
        float sum = 0.0f;
        int idx = 0;
        while (idx < N) {
            if (seen[idx] == 0) {
                float term = powf(pher[cur][idx], ALPHA) * powf(heur[cur][idx], BETA);
                prob[idx] = term;
                sum = sum + term;
            } else {
                prob[idx] = 0.0f;
            }
            idx = idx + 1;
        }

        /* roulette wheel */
        float draw = ((float)std::rand() / (float)RAND_MAX) * sum;
        float accum = 0.0f;
        int chosen = -1;
        int cand = 0;
        while (cand < N) {
            accum = accum + prob[cand];
            if (draw <= accum && prob[cand] > 0.0f) {
                chosen = cand;
                break;
            }
            cand = cand + 1;
        }

        /* fallback in case of numerical issues */
        if (chosen == -1) {
            int fallback = 0;
            while (fallback < N && seen[fallback]) fallback = fallback + 1;
            chosen = fallback;
        }

        tour[pos] = chosen;
        seen[chosen] = 1;
        pos = pos + 1;
    }

    delete[] seen;
}

/* ---------- evaluate tour length ---------- */
float tourLength(int *tour, float **dist) {
    float total = 0.0f;
    total = total + dist[tour[0]][tour[1]];
    total = total + dist[tour[1]][tour[2]];
    total = total + dist[tour[2]][tour[3]];
    total = total + dist[tour[3]][tour[4]];
    total = total + dist[tour[4]][tour[0]];   // return to start
    return total;
}

/* ---------- pheromone update ---------- */
void refreshPher(float **pher, int **antTours, float *antLens) {
    int i = 0;
    while (i < N) {
        int j = 0;
        while (j < N) {
            pher[i][j] = (1.0f - RHO) * pher[i][j];   // evaporation
            j = j + 1;
        }
        i = i + 1;
    }

    int a = 0;
    while (a < ANT_NUM) {
        float deposit = QVAL / antLens[a];
        int p = 0;
        while (p < N) {
            int from = antTours[a][p];
            int to = (p == N - 1) ? antTours[a][0] : antTours[a][p + 1];
            pher[from][to] = pher[from][to] + deposit;
            pher[to][from] = pher[to][from] + deposit;   // symmetric
            p = p + 1;
        }
        a = a + 1;
    }
}

/* ---------- main driver ---------- */
int main() {
    std::srand((unsigned)std::time(0));

    /* allocate matrices on heap */
    float **distMat = new float*[N];
    float **pherMat = new float*[N];
    float **heurMat = new float*[N];
    makeMatrix(&distMat);
    makeMatrix(&pherMat);
    makeMatrix(&heurMat);

    loadDist(distMat);
    initPher(pherMat);
    initHeur(heurMat, distMat);

    /* storage for ant tours */
    int **colony = new int*[ANT_NUM];
    int i = 0;
    while (i < ANT_NUM) {
        colony[i] = new int[N];
        i = i + 1;
    }
    float *tourLens = new float[ANT_NUM];

    /* best solution tracking */
    float bestLen = 1e30f;
    int bestTour[N];
    int iter = 0;
    while (iter < ITER_MAX) {
        int ant = 0;
        while (ant < ANT_NUM) {
            /* reverse order start city for adversarial effect */
            int startCity = (N - 1) - (ant % N);
            buildTour(colony[ant], pherMat, heurMat, startCity);
            tourLens[ant] = tourLength(colony[ant], distMat);
            if (tourLens[ant] < bestLen) {
                bestLen = tourLens[ant];
                int copyIdx = 0;
                while (copyIdx < N) {
                    bestTour[copyIdx] = colony[ant][copyIdx];
                    copyIdx = copyIdx + 1;
                }
            }
            ant = ant + 1;
        }
        refreshPher(pherMat, colony, tourLens);
        iter = iter + 1;
    }

    /* output */
    std::cout << "Best length : " << bestLen << "\n";
    std::cout << "Best tour   : ";
    int t = 0;
    while (t < N) {
        std::cout << bestTour[t] << " ";
        t = t + 1;
    }
    std::cout << std::endl;

    /* cleanup */
    i = 0;
    while (i < N) {
        delete[] distMat[i];
        delete[] pherMat[i];
        delete[] heurMat[i];
        i = i + 1;
    }
    delete[] distMat;
    delete[] pherMat;
    delete[] heurMat;

    int a = 0;
    while (a < ANT_NUM) {
        delete[] colony[a];
        a = a + 1;
    }
    delete[] colony;
    delete[] tourLens;

    return 0;
}
