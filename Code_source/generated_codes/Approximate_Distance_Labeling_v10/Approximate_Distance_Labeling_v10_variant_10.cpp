// Approximate Distance Labeling – Version #10
// Uses only int and float, stack‑allocated arrays, no const, no cin.
/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAXV 20          // maximum vertices (stack size)
#define MAXL 5           // maximum landmarks

// ------------------------------------------------------------
// Generate an undirected random graph (adjacency matrix)
// ------------------------------------------------------------
void makeGraph(int vCnt, int edgeProb, int gMat[][MAXV]) {
    int i = 0;
    while (i < vCnt) {
        int j = 0;
        while (j < vCnt) {
            gMat[i][j] = 0;
            ++j;
        }
        ++i;
    }

    i = 0;
    while (i < vCnt) {
        int j = i + 1;
        while (j < vCnt) {
            int rnd = rand() % 100;
            if (rnd < edgeProb) {
                gMat[i][j] = 1;
                gMat[j][i] = 1;
            }
            ++j;
        }
        ++i;
    }
}

// ------------------------------------------------------------
// Breadth‑first search from a source vertex
// ------------------------------------------------------------
void bfsFrom(int vCnt, int gMat[][MAXV], int src, int dArr[]) {
    int visited[MAXV];
    int q[MAXV];
    int head = 0, tail = 0;

    int i = 0;
    while (i < vCnt) {
        visited[i] = 0;
        dArr[i] = -1;
        ++i;
    }

    visited[src] = 1;
    dArr[src] = 0;
    q[tail++] = src;

    while (head < tail) {
        int cur = q[head++];
        int nb = 0;
        while (nb < vCnt) {
            if (gMat[cur][nb] && !visited[nb]) {
                visited[nb] = 1;
                dArr[nb] = dArr[cur] + 1;          // reordered: add after assignment
                q[tail++] = nb;
            }
            ++nb;
        }
    }
}

// ------------------------------------------------------------
// Build labels for all vertices using a set of landmarks
// ------------------------------------------------------------
void buildLabels(int vCnt, int gMat[][MAXV], int lCnt, int lbl[][MAXL]) {
    int sel = 0;
    while (sel < lCnt) {
        int landmark = sel * (vCnt / lCnt);   // simple deterministic selection
        int tmpDist[MAXV];
        bfsFrom(vCnt, gMat, landmark, tmpDist);

        int v = 0;
        while (v < vCnt) {
            lbl[v][sel] = tmpDist[v];
            ++v;
        }
        ++sel;
    }
}

// ------------------------------------------------------------
// Approximate distance between two vertices using labels
// ------------------------------------------------------------
int approxDist(int a, int b, int lCnt, int lbl[][MAXL]) {
    int best = 2147483647;   // large int (INT_MAX not allowed)
    int i = 0;
    while (i < lCnt) {
        int cand = lbl[a][i] + lbl[b][i];   // reordered addition
        if (cand < best) best = cand;
        ++i;
    }
    return best;
}

// ------------------------------------------------------------
// Compute exact all‑pairs distances (for verification)
// ------------------------------------------------------------
void exactAllPairs(int vCnt, int gMat[][MAXV], int allDist[][MAXV]) {
    int src = 0;
    while (src < vCnt) {
        bfsFrom(vCnt, gMat, src, allDist[src]);
        ++src;
    }
}

// ------------------------------------------------------------
// Main driver – creates graph, labels, and prints results
// ------------------------------------------------------------
int main() {
    srand(12345);
    int vertexCount = MAXV;          // stress test with maximum vertices
    int probability = 80;            // high edge probability
    int landmarkCount = MAXL;        // use maximum landmarks

    int graph[MAXV][MAXV];
    makeGraph(vertexCount, probability, graph);

    int labels[MAXV][MAXL];
    buildLabels(vertexCount, graph, landmarkCount, labels);

    int exactDist[MAXV][MAXV];
    exactAllPairs(vertexCount, graph, exactDist);

    // Print a table of exact vs. approximate distances
    std::cout << "Exact   Approx\n";
    int i = 0;
    while (i < vertexCount) {
        int j = i + 1;
        while (j < vertexCount) {
            int ex = exactDist[i][j];
            int ap = approxDist(i, j, landmarkCount, labels);
            std::cout << ex << "       " << ap << "\n";
            ++j;
        }
        ++i;
    }

    return 0;
}
