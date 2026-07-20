/* LLM input variant 1: minimal-boundary */
/* Approximate Distance Labeling – version #7
 *
 * This program builds a small undirected graph, creates a set of landmarks,
 * computes exact shortest‑path distances, builds approximate labels for every
 * vertex, and finally prints a comparison of exact vs. approximate distances.
 *
 * Restrictions:
 *   – Only int and float are used (no double, long, unsigned, const).
 *   – No standard input (cin); the test graph is generated inside the code.
 *   – All data structures are std::vector based.
 *   – The implementation is class based and written step‑by‑step.
 */

#include <iostream>
#include <vector>
#include <queue>
#include <limits>

class ApproxDistLabeler {
public:
    /* Constructor: builds a deterministic test graph. */
    ApproxDistLabeler()
        : vertexCount(2)               // minimal non‑trivial graph: vertices 0 … 1
    {
        /* Step 1 – initialise adjacency list with the required size. */
        adjacency.assign(vertexCount, std::vector<int>());

        /* Step 2 – add edges (undirected). */
        addEdge(0, 1);                 // single edge connecting the two vertices

        /* Step 3 – choose landmarks (first vertex). */
        landmarkCount = 1;
        landmarks.assign(landmarkCount, 0);
        int i = 0;
        while (i < landmarkCount) {
            landmarks[i] = i;          // landmarks[0]=0
            i = i + 1;
        }

        /* Step 4 – compute exact distances for validation. */
        computeExactDistances();

        /* Step 5 – compute label matrix (distance to each landmark). */
        computeLabels();
    }

    /* Public driver that prints the comparison tables. */
    void runDemo()
    {
        std::cout << "Exact distance matrix:\n";
        printMatrix(exactDist);

        std::cout << "\nApproximate distance matrix (using landmarks):\n";
        std::vector<std::vector<int>> approx = buildApproxMatrix();
        printMatrix(approx);
    }

private:
    int vertexCount;                                   // number of vertices
    std::vector<std::vector<int>> adjacency;           // adjacency list
    int landmarkCount;                                 // how many landmarks
    std::vector<int> landmarks;                        // list of landmark ids
    std::vector<std::vector<int>> label;               // label[vertex][landmark]
    std::vector<std::vector<int>> exactDist;           // exact distances

    /* Helper: add an undirected edge to the adjacency list. */
    void addEdge(int a, int b)
    {
        adjacency[a].push_back(b);
        adjacency[b].push_back(a);
    }

    /* BFS from a source vertex, returns distance vector (int). */
    std::vector<int> bfsFrom(int source)
    {
        std::vector<int> dist(vertexCount, std::numeric_limits<int>::max());
        std::queue<int> q;
        dist[source] = 0;
        q.push(source);

        while (!q.empty()) {
            int cur = q.front(); q.pop();
            int idx = 0;
            while (idx < static_cast<int>(adjacency[cur].size())) {
                int nb = adjacency[cur][idx];
                if (dist[nb] == std::numeric_limits<int>::max()) {
                    dist[nb] = dist[cur] + 1;               // fused: addition inside assignment
                    q.push(nb);
                }
                idx = idx + 1;
            }
        }
        return dist;
    }

    /* Compute exact all‑pairs shortest paths using BFS from each vertex. */
    void computeExactDistances()
    {
        exactDist.assign(vertexCount, std::vector<int>(vertexCount, 0));
        int src = 0;
        while (src < vertexCount) {
            std::vector<int> d = bfsFrom(src);
            int dst = 0;
            while (dst < vertexCount) {
                exactDist[src][dst] = d[dst];
                dst = dst + 1;
            }
            src = src + 1;
        }
    }

    /* Compute the label matrix: distance from every vertex to every landmark. */
    void computeLabels()
    {
        label.assign(vertexCount, std::vector<int>(landmarkCount, 0));
        int lmIdx = 0;
        while (lmIdx < landmarkCount) {
            int lm = landmarks[lmIdx];
            std::vector<int> dFromLandmark = bfsFrom(lm);
            int v = 0;
            while (v < vertexCount) {
                label[v][lmIdx] = dFromLandmark[v];
                v = v + 1;
            }
            lmIdx = lmIdx + 1;
        }
    }

    /* Approximate distance between two vertices using the landmark labels. */
    int approxDist(int u, int v)
    {
        int best = std::numeric_limits<int>::max();
        int i = 0;
        while (i < landmarkCount) {
            int cand = label[u][i] + label[v][i];          // fused addition
            if (cand < best) best = cand;
            i = i + 1;
        }
        return best;
    }

    /* Build full approximate distance matrix for all vertex pairs. */
    std::vector<std::vector<int>> buildApproxMatrix()
    {
        std::vector<std::vector<int>> mat(vertexCount, std::vector<int>(vertexCount, 0));
        int a = 0;
        while (a < vertexCount) {
            int b = 0;
            while (b < vertexCount) {
                mat[a][b] = approxDist(a, b);
                b = b + 1;
            }
            a = a + 1;
        }
        return mat;
    }

    /* Pretty‑print a square integer matrix. */
    void printMatrix(const std::vector<std::vector<int>>& m)
    {
        int r = 0;
        while (r < vertexCount) {
            int c = 0;
            while (c < vertexCount) {
                std::cout << m[r][c];
                if (c < vertexCount - 1) std::cout << ' ';
                c = c + 1;
            }
            std::cout << '\n';
            r = r + 1;
        }
    }
};

/* Entry point – no external input, everything is deterministic. */
int main()
{
    ApproxDistLabeler demo;
    demo.runDemo();
    return 0;
}
