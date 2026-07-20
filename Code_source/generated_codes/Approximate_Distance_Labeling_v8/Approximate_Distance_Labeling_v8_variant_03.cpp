/* LLM input variant 3: zeros-and-ones */
// Approximate Distance Labeling – version #8
// ------------------------------------------------------------
// This program creates a random undirected graph, builds
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
// Helper: generate a random graph on the heap.
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
    int idx = 0;
    while (idx < nodeCnt) {
        degArr[idx] = 0;
        idx = idx + 1;
    }

    // temporary edge list (pairs of vertices)
    int* tmpU = new int[edgeCnt];
    int* tmpV = new int[edgeCnt];
    idx = 0;
    while (idx < edgeCnt) {
        int a = std::rand() % nodeCnt;
        int b = std::rand() % nodeCnt;
        // avoid self‑loops and duplicate edges
        if (a != b) {
            bool duplicated = false;
            int j = 0;
            while (j < idx) {
                if ((tmpU[j] == a && tmpV[j] == b) ||
                    (tmpU[j] == b && tmpV[j] == a)) {
                    duplicated = true;
                    break;
                }
                j = j + 1;
            }
            if (!duplicated) {
                tmpU[idx] = a;
                tmpV[idx] = b;
                // increase degree counters for both ends
                degArr[a] = degArr[a] + 1;
                degArr[b] = degArr[b] + 1;
                idx = idx + 1;
            }
        }
    }

    // compute offsets (prefix sums) for adjacency array
    offArr = new int[nodeCnt + 1];
    offArr[0] = 0;
    idx = 0;
    while (idx < nodeCnt) {
        offArr[idx + 1] = offArr[idx] + degArr[idx];
        idx = idx + 1;
    }

    // allocate adjacency storage
    int totalAdj = offArr[nodeCnt];
    adjArr = new int[totalAdj];

    // fill adjacency list
    // temporary counters to know where to write next neighbor
    int* curPos = new int[nodeCnt];
    idx = 0;
    while (idx < nodeCnt) {
        curPos[idx] = offArr[idx];
        idx = idx + 1;
    }
    idx = 0;
    while (idx < edgeCnt) {
        int u = tmpU[idx];
        int v = tmpV[idx];
        // store v in u's list
        adjArr[curPos[u]] = v;
        curPos[u] = curPos[u] + 1;
        // store u in v's list
        adjArr[curPos[v]] = u;
        curPos[v] = curPos[v] + 1;
        idx = idx + 1;
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
    int i = 0;
    while (i < nodeCnt) {
        dist[i] = nodeCnt + 5;            // sentinel (larger than any possible distance)
        i = i + 1;
    }

    // queue implemented with two indices on a flat array
    int* q = new int[nodeCnt];
    int head = 0, tail = 0;

    // start from source
    dist[src] = 0;
    q[tail] = src;
    tail = tail + 1;

    // BFS loop
    while (head != tail) {
        int cur = q[head];
        head = head + 1;

        // iterate over neighbours of cur
        int start = offArr[cur];
        int finish = offArr[cur + 1];
        int pos = start;
        while (pos < finish) {
            int nb = adjArr[pos];
            // if not visited yet
            if (dist[nb] > nodeCnt) {
                // distance = current distance + 1 (reordered addition)
                dist[nb] = 1 + dist[cur];
                q[tail] = nb;
                tail = tail + 1;
            }
            pos = pos + 1;
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

    int l = 0;
    while (l < labCnt) {
        int lm = landmarks[l];
        // compute exact distances from this landmark
        int* dists = bfsFromSource(lm, nodeCnt, degArr, adjArr, offArr);
        // copy distances into the l‑th column of label matrix
        int v = 0;
        while (v < nodeCnt) {
            // store at index: v * labCnt + l
            label[v * labCnt + l] = dists[v];
            v = v + 1;
        }
        delete[] dists;
        l = l + 1;
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
    int k = 0;
    while (k < labCnt) {
        // retrieve distances from flat matrix
        int du = label[u * labCnt + k];
        int dv = label[v * labCnt + k];
        // sum (reordered)
        int cand = du + dv;
        if (cand < best) {
            best = cand;
        }
        k = k + 1;
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
    int src = 0;
    while (src < nodeCnt) {
        int* d = bfsFromSource(src, nodeCnt, degArr, adjArr, offArr);
        int dst = 0;
        while (dst < nodeCnt) {
            allDist[src * nodeCnt + dst] = d[dst];
            dst = dst + 1;
        }
        delete[] d;
        src = src + 1;
    }
    return allDist;
}

// ------------------------------------------------------------
// Main driver – creates graph, builds labels, answers queries.
// ------------------------------------------------------------
int main()
{
    std::srand(42);                 // deterministic random seed

    // ----- 1. deterministic tiny graph emphasizing zeros and ones -----
    const int V = 2;                // two vertices: 0 and 1
    const int E = 1;                // single edge between them
    int* degree = new int[V];
    degree[0] = 1;
    degree[1] = 1;

    int* offset = new int[V + 1];
    offset[0] = 0;
    offset[1] = 1;
    offset[2] = 2;   // total adjacency entries

    int* adjacency = new int[2];
    adjacency[0] = 1;   // neighbor of 0
    adjacency[1] = 0;   // neighbor of 1

    // ----- 2. deterministic landmark (identity) -----
    const int L = 1;                // one landmark
    int* lands = new int[L];
    lands[0] = 0;                    // landmark is vertex 0

    // ----- 3. build distance labels -----
    int* labelMat = buildLabels(lands, L, V, degree, adjacency, offset);

    // ----- 4. compute exact distances for reference -----
    int* exactMat = computeExactAllPairs(V, degree, adjacency, offset);

    // ----- 5. output results -----
    std::cout << "Landmarks (indices): ";
    int l = 0;
    while (l < L) {
        std::cout << lands[l] << (l + 1 == L ? "\n" : " ");
        l = l + 1;
    }

    std::cout << "\nApproximate distance matrix (using labels):\n";
    int a = 0;
    while (a < V) {
        int b = 0;
        while (b < V) {
            int apr = approxDist(a, b, labelMat, V, L);
            // if nodes are same, distance is zero
            if (a == b) apr = 0;
            std::cout << apr << (b + 1 == V ? "\n" : " ");
            b = b + 1;
        }
        a = a + 1;
    }

    std::cout << "\nExact distance matrix (BFS):\n";
    int p = 0;
    while (p < V) {
        int q = 0;
        while (q < V) {
            int ex = exactMat[p * V + q];
            // sentinel values become -1 for clarity
            if (ex > V) ex = -1;
            std::cout << ex << (q + 1 == V ? "\n" : " ");
            q = q + 1;
        }
        p = p + 1;
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
