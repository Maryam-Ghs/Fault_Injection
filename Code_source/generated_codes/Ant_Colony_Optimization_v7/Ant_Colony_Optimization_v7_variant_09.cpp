/* LLM input variant 9: medium-deterministic-random */
// Ant Colony Optimization - Version 7
// Simple TSP solver using only int and float, heap allocation, and iterative loops.

#include <iostream>
#include <cmath>
#include <cstdlib>

// ---------- Helper Functions ----------
int* generateCities(int nodeCnt) {
    // Deterministic symmetric distance matrix with varied values (0, 1, and larger numbers)
    int* distMat = new int[nodeCnt * nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) {
        for (int j = 0; j < nodeCnt; ++j) {
            if (i == j) {
                distMat[i * nodeCnt + j] = 0;
            } else if (i < j) {
                // Base deterministic value
                int base = ((i * 31 + j * 17) % 997) + 3; // range [3,999]
                // Introduce occasional edge cases
                if ((i + j) % 7 == 0) base = 0;           // zero distance edge
                else if ((i + j) % 5 == 0) base = 1;    // minimal distance
                distMat[i * nodeCnt + j] = base;
                distMat[j * nodeCnt + i] = base;        // symmetrize
            }
        }
    }
    return distMat;
}

float* initPheromone(int nodeCnt, float initVal) {
    float* pher = new float[nodeCnt * nodeCnt];
    for (int i = 0; i < nodeCnt * nodeCnt; ++i) pher[i] = initVal;
    return pher;
}

float* buildHeuristic(const int* distMat, int nodeCnt) {
    float* heur = new float[nodeCnt * nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) {
        for (int j = 0; j < nodeCnt; ++j) {
            if (i == j) {
                heur[i * nodeCnt + j] = 0.0f;
            } else {
                int d = distMat[i * nodeCnt + j];
                heur[i * nodeCnt + j] = (d == 0) ? 1e6f : 1.0f / static_cast<float>(d);
            }
        }
    }
    return heur;
}

// Construct a tour for a single ant
void constructTour(int antIdx, const float* pher, const float* heur,
                   const int* distMat, int nodeCnt,
                   int* tour, float* tourLen, float alpha, float beta) {
    bool* visited = new bool[nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) visited[i] = false;

    int cur = antIdx % nodeCnt;          // start city varies per ant
    tour[0] = cur;
    visited[cur] = true;
    float length = 0.0f;

    for (int step = 1; step < nodeCnt; ++step) {
        // Compute selection probabilities
        float denom = 0.0f;
        for (int cand = 0; cand < nodeCnt; ++cand) {
            if (!visited[cand]) {
                float tau = std::pow(pher[cur * nodeCnt + cand], alpha);
                float eta = std::pow(heur[cur * nodeCnt + cand], beta);
                denom += tau * eta;
            }
        }

        // Roulette wheel selection
        float pick = (static_cast<float>(std::rand()) / RAND_MAX) * denom;
        float accum = 0.0f;
        int chosen = -1;
        for (int cand = 0; cand < nodeCnt; ++cand) {
            if (!visited[cand]) {
                float tau = std::pow(pher[cur * nodeCnt + cand], alpha);
                float eta = std::pow(heur[cur * nodeCnt + cand], beta);
                accum += tau * eta;
                if (accum >= pick) {
                    chosen = cand;
                    break;
                }
            }
        }
        // Fallback (should not happen)
        if (chosen == -1) {
            for (int cand = 0; cand < nodeCnt; ++cand) {
                if (!visited[cand]) { chosen = cand; break; }
            }
        }

        // Update tour
        length += static_cast<float>(distMat[cur * nodeCnt + chosen]);
        cur = chosen;
        tour[step] = cur;
        visited[cur] = true;
    }
    // Return to start
    length += static_cast<float>(distMat[cur * nodeCnt + tour[0]]);
    *tourLen = length;

    delete[] visited;
}

// Update pheromone trails globally
void evaporateAndDeposit(float* pher, const int* allTours,
                        const float* allLengths, int antCnt,
                        int nodeCnt, float evap, float Q) {
    // Evaporation
    for (int i = 0; i < nodeCnt * nodeCnt; ++i) {
        pher[i] = (1.0f - evap) * pher[i];
    }

    // Deposit
    for (int a = 0; a < antCnt; ++a) {
        float contribution = Q / allLengths[a];
        const int* tour = allTours + a * nodeCnt;
        for (int k = 0; k < nodeCnt; ++k) {
            int i = tour[k];
            int j = tour[(k + 1) % nodeCnt];
            pher[i * nodeCnt + j] += contribution;
            pher[j * nodeCnt + i] += contribution; // symmetric TSP
        }
    }
}

// Main ACO driver
void runACO(const int* distMat, int nodeCnt,
            int antCnt, int iterCnt,
            float alpha, float beta,
            float evap, float Q,
            int* bestTour, float* bestLen) {
    // Allocate structures
    float* pher = initPheromone(nodeCnt, 1.0f);
    float* heur = buildHeuristic(distMat, nodeCnt);
    int* tourBuffer = new int[antCnt * nodeCnt];
    float* lengthBuffer = new float[antCnt];

    *bestLen = 1e9f;

    for (int it = 0; it < iterCnt; ++it) {
        // Each ant builds a tour
        for (int a = 0; a < antCnt; ++a) {
            int* curTour = tourBuffer + a * nodeCnt;
            float curLen;
            constructTour(a, pher, heur, distMat, nodeCnt,
                          curTour, &curLen, alpha, beta);
            lengthBuffer[a] = curLen;

            // Keep global best
            if (curLen < *bestLen) {
                *bestLen = curLen;
                for (int i = 0; i < nodeCnt; ++i) bestTour[i] = curTour[i];
            }
        }
        // Pheromone update
        evaporateAndDeposit(pher, tourBuffer, lengthBuffer,
                            antCnt, nodeCnt, evap, Q);
    }

    // Clean up
    delete[] pher;
    delete[] heur;
    delete[] tourBuffer;
    delete[] lengthBuffer;
}

// ---------- Main ----------
int main() {
    // Deterministic seed for any remaining std::rand usage (e.g., selection)
    std::srand(42);

    // Medium‑size problem definition
    const int cityCount = 10;               // larger than original but still manageable
    int* distanceMatrix = generateCities(cityCount);

    // ACO parameters (chosen to stay within int/float)
    const int antNumber = 20;
    const int iterationNumber = 80;
    const float alpha = 1.0f;              // pheromone importance
    const float beta = 2.0f;               // heuristic importance
    const float evaporation = 0.3f;
    const float Q = 100.0f;

    // Result containers
    int* bestPath = new int[cityCount];
    float bestDistance;

    // Run the algorithm
    runACO(distanceMatrix, cityCount,
           antNumber, iterationNumber,
           alpha, beta,
           evaporation, Q,
           bestPath, &bestDistance);

    // Output
    std::cout << "Best tour length: " << bestDistance << "\n";
    std::cout << "Tour order: ";
    for (int i = 0; i < cityCount; ++i) {
        std::cout << bestPath[i] << (i + 1 == cityCount ? "\n" : " -> ");
    }

    // Clean up
    delete[] distanceMatrix;
    delete[] bestPath;
    return 0;
}
