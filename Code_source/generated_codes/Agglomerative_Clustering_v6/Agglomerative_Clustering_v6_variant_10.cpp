#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>        // only for sqrt, still uses float internally

/* LLM input variant 10: large-safe-stress */

/* -------------------------------------------------------------
   Agglomerative Clustering – version #6
   -------------------------------------------------------------
   - Only int and float types are used (no double, long, unsigned, const)
   - All data are allocated on the heap with new / delete
   - Input data are generated inside the program (large predefined set)
   - Verbose step‑by‑step output with expanded intermediate calculations
   - Class‑based implementation, fully self‑contained
   ------------------------------------------------------------- */

class AgglomerativeClustering {
public:
    // constructor: receives number of points, dimension and flat array of coordinates
    AgglomerativeClustering(int pointCount, int dim, float* flatData) {
        nPoints   = pointCount;          // total points
        nDim      = dim;                 // coordinate dimension (here 2)
        // allocate and copy point data onto the heap
        data = new float[nPoints * nDim];
        int i = 0;
        while (i < nPoints * nDim) {
            data[i] = flatData[i];
            ++i;
        }

        // initially each point is its own cluster
        int j = 0;
        while (j < nPoints) {
            std::vector<int> single;
            single.push_back(j);
            clusters.push_back(single);
            ++j;
        }

        // allocate maximum possible distance matrix (nPoints x nPoints)
        maxSize = nPoints;
        distMat = new float[maxSize * maxSize];
        // initialize with a large positive value
        int k = 0;
        while (k < maxSize * maxSize) {
            distMat[k] = 1e30f;
            ++k;
        }
    }

    // destructor: free heap memory
    ~AgglomerativeClustering() {
        delete [] data;
        delete [] distMat;
    }

    // run clustering until the desired number of clusters is reached
    void run(int targetClusters) {
        computeInitialDistances();
        while ((int)clusters.size() > targetClusters) {
            int idxA = -1;
            int idxB = -1;
            findClosestPair(idxA, idxB);
            if (idxA < 0 || idxB < 0) break;   // safety
            mergeTwoClusters(idxA, idxB);
            updateDistanceMatrix(idxA, idxB);
        }
        printResult();
    }

private:
    int nPoints;                // number of original points
    int nDim;                   // dimensionality (2 for this demo)
    int maxSize;                // current matrix dimension (starts at nPoints)
    float* data;                // flat array [point][dim]
    float* distMat;             // distance matrix stored row‑major
    std::vector< std::vector<int> > clusters;   // list of clusters, each holds point indices

    // -----------------------------------------------------------------
    // Compute Euclidean distance between two points (expanded steps)
    // -----------------------------------------------------------------
    float euclideanDist(int pIdx, int qIdx) {
        int baseP = pIdx * nDim;
        int baseQ = qIdx * nDim;
        float diff0 = data[baseP]     - data[baseQ];
        float diff1 = data[baseP + 1] - data[baseQ + 1];
        float sq0  = diff0 * diff0;
        float sq1  = diff1 * diff1;
        float sum  = sq0 + sq1;
        float result = sqrtf(sum);
        return result;
    }

    // -----------------------------------------------------------------
    // Fill the distance matrix with the distances between every pair
    // of initial singleton clusters (single‑linkage)
    // -----------------------------------------------------------------
    void computeInitialDistances() {
        std::cout << "=== Initial distance matrix ===\n";
        int i = 0;
        while (i < nPoints) {
            int j = i + 1;
            while (j < nPoints) {
                float d = euclideanDist(i, j);
                // store symmetrically
                distMat[i * maxSize + j] = d;
                distMat[j * maxSize + i] = d;
                ++j;
            }
            ++i;
        }
        printDistanceMatrix();
    }

    // -----------------------------------------------------------------
    // Locate the two clusters with the smallest distance
    // -----------------------------------------------------------------
    void findClosestPair(int& outIdxA, int& outIdxB) {
        float best = 1e30f;
        int a = -1;
        int b = -1;
        int i = 0;
        while (i < (int)clusters.size()) {
            int j = i + 1;
            while (j < (int)clusters.size()) {
                // distance between clusters i and j is stored at [i][j]
                float cur = distMat[i * maxSize + j];
                if (cur < best) {
                    best = cur;
                    a = i;
                    b = j;
                }
                ++j;
            }
            ++i;
        }
        outIdxA = a;
        outIdxB = b;
        std::cout << "Closest pair: Cluster " << a << " and Cluster " << b
                  << " with distance " << best << "\n";
    }

    // -----------------------------------------------------------------
    // Merge cluster B into cluster A, erase B from the list
    // -----------------------------------------------------------------
    void mergeTwoClusters(int idxA, int idxB) {
        std::cout << "Merging Cluster " << idxB << " into Cluster " << idxA << "\n";
        // append all points of B to A
        int p = 0;
        while (p < (int)clusters[idxB].size()) {
            clusters[idxA].push_back(clusters[idxB][p]);
            ++p;
        }
        // erase B (maintain order by swapping with last then pop_back)
        int lastIdx = (int)clusters.size() - 1;
        if (idxB != lastIdx) {
            clusters[idxB] = clusters[lastIdx];
            // also need to move the corresponding rows/columns in distance matrix
            copyRowCol(lastIdx, idxB);
        }
        clusters.pop_back();
        // shrink effective matrix size if needed
        maxSize = (int)clusters.size();
    }

    // -----------------------------------------------------------------
    // Helper: copy distance matrix row/column from src to dst (used after swap)
    // -----------------------------------------------------------------
    void copyRowCol(int src, int dst) {
        int i = 0;
        while (i < maxSize) {
            // copy row
            distMat[dst * maxSize + i] = distMat[src * maxSize + i];
            // copy column
            distMat[i * maxSize + dst] = distMat[i * maxSize + src];
            ++i;
        }
        // set the old src row/col to a large value (will be ignored)
        i = 0;
        while (i < maxSize) {
            distMat[src * maxSize + i] = 1e30f;
            distMat[i * maxSize + src] = 1e30f;
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // After a merge, recompute distances from the new cluster (idxA)
    // to every other remaining cluster using single‑linkage rule
    // -----------------------------------------------------------------
    void updateDistanceMatrix(int idxA, int mergedIdx) {
        // mergedIdx is the index that was removed (now contains old last cluster)
        // compute new distances for idxA against all others
        int i = 0;
        while (i < (int)clusters.size()) {
            if (i == idxA) {
                ++i;
                continue;
            }
            // single linkage: min distance between any point in cluster A and any point in cluster i
            float minDist = 1e30f;
            int p = 0;
            while (p < (int)clusters[idxA].size()) {
                int q = 0;
                while (q < (int)clusters[i].size()) {
                    float d = euclideanDist(clusters[idxA][p], clusters[i][q]);
                    if (d < minDist) minDist = d;
                    ++q;
                }
                ++p;
            }
            // store symmetrically
            distMat[idxA * maxSize + i] = minDist;
            distMat[i * maxSize + idxA] = minDist;
            ++i;
        }
        // Invalidate distances that involved the removed cluster
        int r = 0;
        while (r < maxSize + 1) {   // +1 because we still have old maxSize before decrement
            distMat[mergedIdx * maxSize + r] = 1e30f;
            distMat[r * maxSize + mergedIdx] = 1e30f;
            ++r;
        }
        std::cout << "Distance matrix updated after merge.\n";
        printDistanceMatrix();
    }

    // -----------------------------------------------------------------
    // Pretty‑print the current distance matrix (only the active part)
    // -----------------------------------------------------------------
    void printDistanceMatrix() {
        int sz = (int)clusters.size();
        std::cout << "   ";
        int h = 0;
        while (h < sz) {
            std::cout << std::setw(8) << h;
            ++h;
        }
        std::cout << "\n";
        int i = 0;
        while (i < sz) {
            std::cout << std::setw(3) << i << " ";
            int j = 0;
            while (j < sz) {
                float v = distMat[i * maxSize + j];
                if (i == j) std::cout << std::setw(8) << "0";
                else        std::cout << std::setw(8) << std::fixed << std::setprecision(3) << v;
                ++j;
            }
            std::cout << "\n";
            ++i;
        }
        std::cout << "\n";
    }

    // -----------------------------------------------------------------
    // Final result: print each cluster and its member points
    // -----------------------------------------------------------------
    void printResult() {
        std::cout << "=== Final clusters ===\n";
        int c = 0;
        while (c < (int)clusters.size()) {
            std::cout << "Cluster " << c << " (size " << (int)clusters[c].size() << "): ";
            int p = 0;
            while (p < (int)clusters[c].size()) {
                int idx = clusters[c][p];
                int base = idx * nDim;
                std::cout << "[" << data[base] << "," << data[base + 1] << "]";
                if (p < (int)clusters[c].size() - 1) std::cout << ", ";
                ++p;
            }
            std::cout << "\n";
            ++c;
        }
    }
};

/* -------------------------------------------------------------
   Main – generate a larger dataset and run the clustering
   ------------------------------------------------------------- */
int main() {
    // larger predefined array: 200 points in 2‑D space arranged in a grid (20x10)
    int pointCount = 200;
    int dimension  = 2;
    float* rawData = new float[pointCount * dimension];
    int i = 0;
    while (i < pointCount) {
        // x coordinate cycles every 20 points, y increases every 20 points
        rawData[i * 2]     = static_cast<float>(i % 20);          // x
        rawData[i * 2 + 1] = static_cast<float>(i / 20);          // y
        ++i;
    }

    // create clustering object
    AgglomerativeClustering ac(pointCount, dimension, rawData);

    // we want to end with 5 clusters
    int target = 5;
    ac.run(target);

    delete [] rawData;
    return 0;
}
