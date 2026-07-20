#include <iostream>
#include <algorithm>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ---------- deterministic graph ----------
    // number of vertices
    int verticesCount = 5;

    // original edge capacities (undirected, stored as directed both ways)
    // duplicate-heavy: many identical capacities (2) and zeros
    int edgeCap[5][5] = {
        {0, 2, 2, 2, 2},
        {2, 0, 2, 2, 2},
        {2, 2, 0, 2, 2},
        {2, 2, 2, 0, 2},
        {2, 2, 2, 2, 0}
    };

    // matrix that will hold the all‑pairs edge connectivities
    int conn[5][5];
    for (int i = 0; i < verticesCount; ++i)
        for (int j = 0; j < verticesCount; ++j)
            conn[i][j] = 0;

    // ---------- main all‑pairs loop ----------
    for (int srcIdx = 0; srcIdx < verticesCount; ++srcIdx) {
        for (int dstIdx = 0; dstIdx < verticesCount; ++dstIdx) {
            if (srcIdx == dstIdx) continue;               // skip same vertex

            // copy original capacities into a mutable residual matrix
            int resCap[5][5];
            for (int a = 0; a < verticesCount; ++a)
                for (int b = 0; b < verticesCount; ++b)
                    resCap[a][b] = edgeCap[a][b];

            int maxFlow = 0;                               // accumulated flow

            // ---------- Edmonds‑Karp iteration ----------
            while (true) {
                // BFS to find an augmenting path
                int bfsQueue[5];
                int qFront = 0, qBack = 0;
                bfsQueue[qBack++] = srcIdx;

                int visitedArr[5] = {0};
                visitedArr[srcIdx] = 1;

                int parentArr[5];
                for (int p = 0; p < verticesCount; ++p) parentArr[p] = -1;

                bool foundPath = false;

                while (qFront < qBack && !foundPath) {
                    int cur = bfsQueue[qFront++];
                    for (int nxt = 0; nxt < verticesCount; ++nxt) {
                        // expanded condition checks
                        int capHere = resCap[cur][nxt];
                        int notVisited = visitedArr[nxt] == 0;
                        int positiveCap = capHere > 0;
                        if (positiveCap && notVisited) {
                            bfsQueue[qBack++] = nxt;
                            visitedArr[nxt] = 1;
                            parentArr[nxt] = cur;
                            if (nxt == dstIdx) {
                                foundPath = true;
                                break;
                            }
                        }
                    }
                }

                if (!foundPath) break;      // no augmenting path left

                // compute bottleneck capacity on the found path
                int pathFlow = 123456789;   // start with a large number
                int step = dstIdx;
                while (step != srcIdx) {
                    int prev = parentArr[step];
                    int edgeCapacity = resCap[prev][step];
                    // multi‑step min update
                    int smaller = edgeCapacity < pathFlow ? edgeCapacity : pathFlow;
                    pathFlow = smaller;
                    step = prev;
                }

                // augment flow along the path
                step = dstIdx;
                while (step != srcIdx) {
                    int prev = parentArr[step];

                    // decrease forward edge
                    int beforeFwd = resCap[prev][step];
                    int afterFwd = beforeFwd - pathFlow;
                    resCap[prev][step] = afterFwd;

                    // increase reverse edge
                    int beforeRev = resCap[step][prev];
                    int afterRev = beforeRev + pathFlow;
                    resCap[step][prev] = afterRev;

                    step = prev;
                }

                // add this path's flow to total
                maxFlow = maxFlow + pathFlow;
            } // end while (augmenting)

            // store the result for this ordered pair
            conn[srcIdx][dstIdx] = maxFlow;
        }
    }

    // ---------- output ----------
    std::cout << "All‑Pairs Edge Connectivity Matrix:\n";
    for (int i = 0; i < verticesCount; ++i) {
        for (int j = 0; j < verticesCount; ++j) {
            std::cout << conn[i][j];
            if (j < verticesCount - 1) std::cout << ' ';
        }
        std::cout << '\n';
    }

    return 0;
}
