/* LLM input variant 6: ordered-structured */
// Approximate Distance Labeling – version #8
// ------------------------------------------------------------
// This program creates a deterministic structured graph, builds
// distance labels using a small set of landmarks, and
// answers distance queries approximately.
// Only int and float types are used. All dynamic memory
// is allocated with new/delete. The code is split into
// helper functions and follows a verbose, step‑by‑step style.
// ------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// ------------------------------------------------------------
// Helper: generate a deterministic structured graph on the heap.
// Parameters:
//   nodeCnt – number of vertices
//   edgeCnt – number of undirected edges
//   degArr  – output array with degree of each vertex
//   adjArr  – output adjacency list (flattened)
//   offArr  – output offset array: start index in adjArr for each vertex
// ------------------------------------------------------------
void generateGraph(int nodeCnt, int edgeCnt,
                   int* &degArr, int* &adjArr, int* &offArr)
{
    // initialise degree array with zeros
    degArr = new int[nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) {
        degArr[i] = 0;
    }

    // temporary edge list (pairs of vertices)
    int* tmpU = new int[edgeCnt];
    int* tmpV = new int[edgeCnt];
    int added = 0;

    // deterministic edge addition: connect vertices with increasing distance
    for (int d = 1; added < edgeCnt && d < nodeCnt; ++d) {
        for (int u = 0; u + d < nodeCnt && added < edgeCnt; ++u) {
            int v = u + d;
            tmpU[added] = u;
            tmpV[added] = v;
            degArr[u] += 1;
            degArr[v] += 1;
            ++added;
        }
    }

    // compute offsets (prefix sums) for adjacency array
    offArr = new int[nodeCnt + 1];
    offArr[0] = 0;
    for (int i = 0; i < nodeCnt; ++i) {
        offArr[i + 1] = offArr[i] + degArr[i];
    }

    // allocate adjacency storage
    int totalAdj = offArr[nodeCnt];
    adjArr = new int[totalAdj];

    // fill adjacency list
    int* curPos = new int[nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) {
        curPos[i] = offArr[i];
    }
    for (int i = 0; i < added; ++i) {
        int u = tmpU[i];
        int v = tmpV[i];
        // store v in u's list
        adjArr[curPos[u]++] = v;
        // store u in v's list
        adjArr[curPos[v]++] = u;
    }

    // ensure each vertex's neighbor list is sorted
    for (int i = 0; i < nodeCnt; ++i) {
        int start = offArr[i];
        int finish = offArr[i + 1];
        std::sort(adjArr + start, adjArr + finish);
    }

    // cleanup temporaries
    delete[] tmpU;
    delete[] tmpV;
    delete[] curPos;
}

// ------------------------------------------------------------
// Helper: Breadth‑First Search from a source vertex.
// Returns an array of distances (size = nodeCnt).
// Unreachable vertices receive a large sentinel value.
// ------------------------------------------------------------
int* bfsFromSource(int src, int nodeCnt,
                   const int* degArr, const int* adjArr, const int* offArr)
{
    // distance array, initialised to a big number
    int* dist = new int[nodeCnt];
    for (int i = 0; i < nodeCnt; ++i) {
        dist[i] = nodeCnt + 5;            // sentinel (larger than any possible distance)
    }

    // queue implemented with two indices on a flat array
    int* q = new int[nodeCnt];
    int head = 0, tail = 0;

    // start from source
    dist[src] = 0;
    q[tail++] = src;

    // BFS loop
    while (head != tail) {
        int cur = q[head++];
        int start = offArr[cur];
        int finish = offArr[cur + 1];
        for (int pos = start; pos < finish; ++pos) {
            int nb = adjArr[pos];
            // if not visited yet
            if (dist[nb] > nodeCnt) {
                // distance = current distance + 1 (reordered addition)
                dist[nb] = 1 + dist[cur];
                q[tail++] = nb;
            }
        }
    }

    delete[] q;
    return dist;
}

// ------------------------------------------------------------
// Helper: Build distance labels for a set of landmarks.
// Parameters:
//   landmarks – array of landmark vertex ids (size = labCnt)
//   labCnt    – number of landmarks
//   nodeCnt   – number of vertices
//   degArr,adjArr,offArr – graph representation
// Returns a 2‑D array label[node][k] = distance(node, landmark_k)
// ------------------------------------------------------------
int* buildLabels(const int* landmarks, int labCnt, int nodeCnt,
                 const int* degArr, const int* adjArr, const int* offArr)
{
    // flat 2‑D array: row major (nodeCnt rows, labCnt columns)
    int* label = new int[nodeCnt * labCnt];

    for (int l = 0; l < labCnt; ++l) {
        int lm = landmarks[l];
        // compute exact distances from this landmark
        int* dists = bfsFromSource(lm, nodeCnt, degArr, adjArr, offArr);
        // copy distances into the l‑th column of label matrix
        for (int v = 0; v < nodeCnt; ++v) {
            label[v * labCnt + l] = dists[v];
        }
        delete[] dists;
    }
    return label;
}

// ------------------------------------------------------------
// Helper: Approximate distance between two vertices using labels.
// Returns the minimum over all landmarks of (label[u][k] + label[v][k]).
// ------------------------------------------------------------
int approxDist(int u, int v, const int* label,
               int nodeCnt, int labCnt)
{
    int best = nodeCnt + 5;   // initialise with sentinel
    for (int k = 0; k < labCnt; ++k) {
        // retrieve distances from flat matrix
        int du = label[u * labCnt + k];
        int dv = label[v * labCnt + k];
        // sum (reordered)
        int cand = du + dv;
        if (cand < best) {
            best = cand;
        }
    }
    return best;
}

// ------------------------------------------------------------
// Helper: Compute exact all‑pairs distances (for verification).
// Uses BFS from every vertex.
// Returns a flat matrix exactDist[i * nodeCnt + j].
// ------------------------------------------------------------
int* computeExactAllPairs(int nodeCnt,
                          const int* degArr, const int* adjArr, const int* offArr)
{
    int* allDist = new int[nodeCnt * nodeCnt];
    for (int src = 0; src < nodeCnt; ++src) {
        int* d = bfsFromSource(src, nodeCnt, degArr, adjArr, offArr);
        for (int dst = 0; dst < nodeCnt; ++dst) {
            allDist[src * nodeCnt + dst] = d[dst];
        }
        delete[] d;
    }
    return allDist;
}

// ------------------------------------------------------------
// Main driver – creates graph, builds labels, answers queries.
// ------------------------------------------------------------
int main()
{
    // deterministic graph, no random seed needed

    // ----- 1. generate a medium‑sized deterministic structured graph -----
    const int V = 12;               // number of vertices (medium)
    const int E = 30;               // number of edges (structured)
    int* degree = nullptr;
    int* adjacency = nullptr;
    int* offset = nullptr;
    generateGraph(V, E, degree, adjacency, offset);

    // ----- 2. select deterministic landmarks -----
    const int L = 3;                // number of landmarks
    int* lands = new int[L];
    // Choose first, middle, and last vertices
    lands[0] = 0;
    lands[1] = V / 2;
    lands[2] = V - 1;

    // ----- 3. build distance labels -----
    int* labelMat = buildLabels(lands, L, V, degree, adjacency, offset);

    // ----- 4. compute exact distances for reference -----
    int* exactMat = computeExactAllPairs(V, degree, adjacency, offset);

    // ----- 5. output results -----
    std::cout << "Landmarks (indices): ";
    for (int l = 0; l < L; ++l) {
        std::cout << lands[l] << (l + 1 == L ? "\n" : " ");
    }

    std::cout << "\nApproximate distance matrix (using labels):\n";
    for (int a = 0; a < V; ++a) {
        for (int b = 0; b < V; ++b) {
            int apr = approxDist(a, b, labelMat, V, L);
            // if nodes are same, distance is zero
            if (a == b) apr = 0;
            std::cout << apr << (b + 1 == V ? "\n" : " ");
        }
    }

    std::cout << "\nExact distance matrix (BFS):\n";
    for (int p = 0; p < V; ++p) {
        for (int q = 0; q < V; ++q) {
            int ex = exactMat[p * V + q];
            // sentinel values become -1 for clarity
            if (ex > V) ex = -1;
            std::cout << ex << (q + 1 == V ? "\n" : " ");
        }
    }

    // ----- 6. clean up heap memory -----
    delete[] degree;
    delete[] adjacency;
    delete[] offset;
    delete[] lands;
    delete[] labelMat;
    delete[] exactMat;

    return 0;
}
