/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <queue>

// ------------------------------------------------------------
// Version #5 – All‑Pairs Shortest Path using Repeated Dijkstra
// ------------------------------------------------------------

class Graph {
public:
    // Number of vertices
    int vertexCount;

    // Adjacency matrix stored on the heap
    int **matrix;

    // Constructor: allocate matrix and fill with zeros
    Graph(int v) {
        vertexCount = v;

        // Allocate rows
        matrix = new int*[vertexCount];
        int rowIdx = 0;
        while (rowIdx < vertexCount) {
            // Allocate each row
            matrix[rowIdx] = new int[vertexCount];
            int colIdx = 0;
            while (colIdx < vertexCount) {
                matrix[rowIdx][colIdx] = 0;   // initialise with 0 (no edge yet)
                colIdx = colIdx + 1;
            }
            rowIdx = rowIdx + 1;
        }
    }

    // Destructor: free all heap memory
    ~Graph() {
        int rowIdx = 0;
        while (rowIdx < vertexCount) {
            delete [] matrix[rowIdx];
            rowIdx = rowIdx + 1;
        }
        delete [] matrix;
    }

    // Populate the matrix with a sparse, skewed deterministic graph
    void buildRandom() {
        const int INF = 1000000;   // a large value representing “no edge”

        // Initialise all off‑diagonal entries to INF (no edge)
        int src = 0;
        while (src < vertexCount) {
            int dst = 0;
            while (dst < vertexCount) {
                if (src == dst) {
                    matrix[src][dst] = 0;                // distance to self is zero
                } else {
                    matrix[src][dst] = INF;              // no direct edge initially
                }
                dst = dst + 1;
            }
            src = src + 1;
        }

        // Create a small dense cluster among vertices 0,1,2
        int cluster[] = {0, 1, 2};
        int clusterSize = 3;
        int i = 0;
        while (i < clusterSize) {
            int j = 0;
            while (j < clusterSize) {
                if (i != j) {
                    int u = cluster[i];
                    int v = cluster[j];
                    // Deterministic weight between 1 and 5
                    int weight = ((u + v) % 5) + 1;
                    matrix[u][v] = weight;
                }
                j = j + 1;
            }
            i = i + 1;
        }

        // Add a few sparse edges connecting the rest of the graph
        // Edge 3 -> 0
        matrix[3][0] = 7;
        // Edge 4 -> 2
        matrix[4][2] = 9;
        // Edge 5 -> 3
        matrix[5][3] = 12;
        // Edge 5 -> 1 (directed)
        matrix[5][1] = 8;
    }

    // Single‑source Dijkstra: compute shortest distances from 'origin'
    void singleSourceDijkstra(int origin, int *distArray) {
        const int INF = 1000000;

        // Step 1 – initialise distance and visited arrays
        int idx = 0;
        while (idx < vertexCount) {
            distArray[idx] = INF;          // unknown distance = INF
            idx = idx + 1;
        }
        distArray[origin] = 0;             // distance to self is zero

        bool *visited = new bool[vertexCount];
        idx = 0;
        while (idx < vertexCount) {
            visited[idx] = false;
            idx = idx + 1;
        }

        // Step 2 – repeat V times: pick the closest unvisited vertex
        int iter = 0;
        while (iter < vertexCount) {
            // Find the unvisited vertex with minimal distance
            int minDist = INF;
            int minVertex = -1;
            int probe = 0;
            while (probe < vertexCount) {
                if (!visited[probe] && distArray[probe] < minDist) {
                    minDist = distArray[probe];
                    minVertex = probe;
                }
                probe = probe + 1;
            }

            // If no reachable vertex remains, break early
            if (minVertex == -1) {
                break;
            }

            // Mark the chosen vertex as visited
            visited[minVertex] = true;

            // Relax edges outgoing from minVertex
            int neighbour = 0;
            while (neighbour < vertexCount) {
                int edgeWeight = matrix[minVertex][neighbour];
                if (edgeWeight != INF) {                     // an edge exists
                    int possibleDist = distArray[minVertex] + edgeWeight;
                    if (possibleDist < distArray[neighbour]) {
                        distArray[neighbour] = possibleDist;
                    }
                }
                neighbour = neighbour + 1;
            }

            iter = iter + 1;
        }

        delete [] visited;
    }

    // Compute all‑pairs shortest paths and store them into 'outputMatrix'
    void computeAllPairs(int **outputMatrix) {
        int src = 0;
        while (src < vertexCount) {
            // Temporary array to hold distances from 'src'
            int *tempDist = new int[vertexCount];
            singleSourceDijkstra(src, tempDist);

            // Copy the result into the appropriate row of outputMatrix
            int dst = 0;
            while (dst < vertexCount) {
                outputMatrix[src][dst] = tempDist[dst];
                dst = dst + 1;
            }

            delete [] tempDist;
            src = src + 1;
        }
    }

    // Pretty‑print a matrix of size V×V
    void displayMatrix(const char *title, int **mat) {
        std::cout << title << std::endl;
        int r = 0;
        while (r < vertexCount) {
            int c = 0;
            while (c < vertexCount) {
                // Align output: replace INF with a dash
                if (mat[r][c] >= 1000000) {
                    std::cout << "  - ";
                } else {
                    // Add extra space for single‑digit numbers
                    if (mat[r][c] < 10) {
                        std::cout << "  " << mat[r][c] << " ";
                    } else {
                        std::cout << " " << mat[r][c] << " ";
                    }
                }
                c = c + 1;
            }
            std::cout << std::endl;
            r = r + 1;
        }
        std::cout << std::endl;
    }
};

int main() {
    // Seed the random generator (not used in this variant but kept for completeness)
    std::srand(static_cast<unsigned>(std::time(0)));

    // ------------------------------------------------
    // 1. Create a graph with a chosen size (medium test)
    // ------------------------------------------------
    int size = 6;                         // moderate number of vertices
    Graph myGraph(size);

    // ------------------------------------------------
    // 2. Build a deterministic sparse, skewed adjacency matrix
    // ------------------------------------------------
    myGraph.buildRandom();

    // ------------------------------------------------
    // 3. Allocate a matrix to hold all‑pairs results
    // ------------------------------------------------
    int **result = new int*[size];
    int row = 0;
    while (row < size) {
        result[row] = new int[size];
        row = row + 1;
    }

    // ------------------------------------------------
    // 4. Run the repeated Dijkstra algorithm
    // ------------------------------------------------
    myGraph.computeAllPairs(result);

    // ------------------------------------------------
    // 5. Show the original adjacency matrix and the shortest‑path matrix
    // ------------------------------------------------
    myGraph.displayMatrix("Adjacency Matrix (INF = no edge):", myGraph.matrix);
    myGraph.displayMatrix("All‑Pairs Shortest Path Matrix:", result);

    // ------------------------------------------------
    // 6. Clean up heap memory
    // ------------------------------------------------
    row = 0;
    while (row < size) {
        delete [] result[row];
        row = row + 1;
    }
    delete [] result;

    return 0;
}
