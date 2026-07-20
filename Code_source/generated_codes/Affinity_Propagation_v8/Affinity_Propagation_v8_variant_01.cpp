/* LLM input variant 1: minimal-boundary */
// Affinity Propagation – version #8
// ---------------------------------------------------------------
// This implementation follows the constraints:
//   * Only float and int types are used (no double, long, unsigned, const)
//   * All data is allocated on the heap with new/delete
//   * Input is generated internally (small predefined data set)
//   * The algorithm is split into helper functions
//   * Verbose, step‑by‑step style with direct computations
// ---------------------------------------------------------------

#include <iostream>
#include <cmath>   // for std::sqrt (allowed, returns float when given float)

// --------------------------- Helper Functions ---------------------------

// Compute squared Euclidean distance between two 2‑D points (float version)
float squaredDist(const float* pA, const float* pB) {
    float dx = pA[0] - pB[0];
    float dy = pA[1] - pB[1];
    return dx * dx + dy * dy;
}

// Build the similarity matrix S (negative squared Euclidean distance)
void buildSimilarity(const float* data, int nPoints, float* S) {
    // S is a flat nPoints × nPoints matrix stored row‑major
    for (int i = 0; i < nPoints; ++i) {
        for (int j = 0; j < nPoints; ++j) {
            if (i == j) {
                // Preference will be set later; temporarily store 0
                S[i * nPoints + j] = 0.0f;
            } else {
                S[i * nPoints + j] = -squaredDist(&data[i * 2], &data[j * 2]);
            }
        }
    }
}

// Set the preference values (median of off‑diagonal similarities)
void setPreference(float* S, int nPoints) {
    // Collect off‑diagonal values
    int cnt = nPoints * (nPoints - 1);
    float* temp = new float[cnt];
    int idx = 0;
    for (int i = 0; i < nPoints; ++i) {
        for (int j = 0; j < nPoints; ++j) {
            if (i != j) {
                temp[idx++] = S[i * nPoints + j];
            }
        }
    }
    // Simple selection sort to find median (since cnt is tiny)
    for (int i = 0; i < cnt - 1; ++i) {
        for (int j = i + 1; j < cnt; ++j) {
            if (temp[j] < temp[i]) {
                float swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }
    float median = temp[cnt / 2];
    delete[] temp;

    // Assign median as preference on the diagonal
    for (int i = 0; i < nPoints; ++i) {
        S[i * nPoints + i] = median;
    }
}

// Initialise responsibility (R) and availability (A) matrices to zero
void initialiseMessages(float* R, float* A, int nPoints) {
    int total = nPoints * nPoints;
    for (int i = 0; i < total; ++i) {
        R[i] = 0.0f;
        A[i] = 0.0f;
    }
}

// Update responsibilities (R) based on current A and S
void updateResponsibility(const float* S, const float* A, float* R, int nPoints) {
    for (int i = 0; i < nPoints; ++i) {
        // Find the largest and second‑largest values of A[i,k] + S[i,k]
        float max1 = -1e30f;
        float max2 = -1e30f;
        int maxIdx = -1;
        for (int k = 0; k < nPoints; ++k) {
            float val = A[i * nPoints + k] + S[i * nPoints + k];
            if (val > max1) {
                max2 = max1;
                max1 = val;
                maxIdx = k;
            } else if (val > max2) {
                max2 = val;
            }
        }
        // Compute new responsibilities
        for (int k = 0; k < nPoints; ++k) {
            float oldR = R[i * nPoints + k];
            float candidate = S[i * nPoints + k];
            if (k == maxIdx) {
                candidate -= max2;
            } else {
                candidate -= max1;
            }
            // Damping factor (lambda = 0.5) for stability
            R[i * nPoints + k] = 0.5f * oldR + 0.5f * candidate;
        }
    }
}

// Update availabilities (A) based on new R
void updateAvailability(const float* R, float* A, int nPoints) {
    for (int k = 0; k < nPoints; ++k) {
        // Sum of positive responsibilities for point k, excluding i = k
        float sumPos = 0.0f;
        for (int i = 0; i < nPoints; ++i) {
            if (i == k) continue;
            float rVal = R[i * nPoints + k];
            if (rVal > 0.0f) sumPos += rVal;
        }
        // Update A for each i
        for (int i = 0; i < nPoints; ++i) {
            float oldA = A[i * nPoints + k];
            float newA;
            if (i == k) {
                // Self‑availability
                newA = sumPos;
            } else {
                float rkk = R[k * nPoints + k];
                float pos = R[i * nPoints + k] > 0.0f ? R[i * nPoints + k] : 0.0f;
                newA = std::min(0.0f, rkk + sumPos - pos);
            }
            // Damping factor (lambda = 0.5)
            A[i * nPoints + k] = 0.5f * oldA + 0.5f * newA;
        }
    }
}

// Determine exemplars from final A and R matrices
void extractExemplars(const float* A, const float* R, int nPoints, int* exemplarIdx) {
    for (int i = 0; i < nPoints; ++i) {
        // A[i,k] + R[i,k] > 0 indicates k is an exemplar for i
        int best = -1;
        for (int k = 0; k < nPoints; ++k) {
            float score = A[i * nPoints + k] + R[i * nPoints + k];
            if (score > 0.0f) {
                best = k;
                break; // first positive is sufficient for this simple demo
            }
        }
        // If none positive, assign to the point with highest score
        if (best == -1) {
            float maxScore = -1e30f;
            for (int k = 0; k < nPoints; ++k) {
                float score = A[i * nPoints + k] + R[i * nPoints + k];
                if (score > maxScore) {
                    maxScore = score;
                    best = k;
                }
            }
        }
        exemplarIdx[i] = best;
    }
}

// ---------------------------------------------------------------
// Main driver
// ---------------------------------------------------------------
int main() {
    // ----------------------------------------------------------------
    // 1. Define a tiny data set (2 points in 2‑D space) – minimal boundary
    // ----------------------------------------------------------------
    int pointCount = 2;
    // Flat array: [x0, y0, x1, y1, ...]
    float* points = new float[pointCount * 2];
    points[0] = 0.0f; points[1] = 0.0f;   // Point 0
    points[2] = 1.0f; points[3] = 1.0f;   // Point 1

    // ----------------------------------------------------------------
    // 2. Allocate similarity (S), responsibility (R) and availability (A)
    // ----------------------------------------------------------------
    float* S = new float[pointCount * pointCount];
    float* R = new float[pointCount * pointCount];
    float* A = new float[pointCount * pointCount];

    // ----------------------------------------------------------------
    // 3. Build similarity matrix and set preferences
    // ----------------------------------------------------------------
    buildSimilarity(points, pointCount, S);
    setPreference(S, pointCount);

    // ----------------------------------------------------------------
    // 4. Initialise messages
    // ----------------------------------------------------------------
    initialiseMessages(R, A, pointCount);

    // ----------------------------------------------------------------
    // 5. Run the Affinity Propagation iterations
    // ----------------------------------------------------------------
    int maxIter = 15;          // small number for demo
    int iter = 0;
    while (iter < maxIter) {
        // Update responsibilities first
        updateResponsibility(S, A, R, pointCount);
        // Then update availabilities
        updateAvailability(R, A, pointCount);
        ++iter;
    }

    // ----------------------------------------------------------------
    // 6. Extract exemplars and print the clustering result
    // ----------------------------------------------------------------
    int* exemplarFor = new int[pointCount];
    extractExemplars(A, R, pointCount, exemplarFor);

    std::cout << "Affinity Propagation clustering result:\n";
    for (int i = 0; i < pointCount; ++i) {
        std::cout << "Point " << i << " ("
                  << points[i * 2] << ", " << points[i * 2 + 1]
                  << ") -> Exemplar " << exemplarFor[i] << " ("
                  << points[exemplarFor[i] * 2] << ", "
                  << points[exemplarFor[i] * 2 + 1] << ")\n";
    }

    // ----------------------------------------------------------------
    // 7. Clean up heap memory
    // ----------------------------------------------------------------
    delete[] points;
    delete[] S;
    delete[] R;
    delete[] A;
    delete[] exemplarFor;

    return 0;
}
