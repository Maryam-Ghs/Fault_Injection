#include <iostream>

/* LLM input variant 8: sparse-skewed */
int main() {
    // version #10
    // sparse, skewed graph (7 vertices, 3 edges clustered among first three vertices)
    int V = 7;
    int E = 3;
    int aU[3] = {0, 0, 1};
    int aV[3] = {1, 2, 2};

    // adjacency matrix on the heap
    int* adj = new int[V * V];
    for (int i = 0; i < V * V; ++i) adj[i] = 0;
    for (int i = 0; i < E; ++i) {
        int x = aU[i];
        int y = aV[i];
        adj[x * V + y] = 1;
        adj[y * V + x] = 1;
    }

    // auxiliary arrays on the heap
    int* disc = new int[V];
    int* low  = new int[V];
    int* par  = new int[V];
    int* seen = new int[V];
    int* art  = new int[V];
    for (int i = 0; i < V; ++i) {
        disc[i] = -1;
        low[i]  = -1;
        par[i]  = -1;
        seen[i] = 0;
        art[i]  = 0;
    }

    int timer = 0;

    struct Frame {
        int vtx;
        int parent;
        int nxtIdx;
        int childCnt;
    };

    // iterative DFS for every component
    for (int start = 0; start < V; ++start) {
        if (seen[start]) continue;

        // stack on the heap
        Frame* stk = new Frame[V];
        int sp = 0;
        stk[sp++] = { start, -1, 0, 0 };

        while (sp) {
            Frame& cur = stk[sp - 1];
            int v = cur.vtx;

            if (!seen[v]) {
                seen[v] = 1;
                disc[v] = low[v] = ++timer;
            }

            bool goDeeper = false;
            for (; cur.nxtIdx < V; ++cur.nxtIdx) {
                int w = cur.nxtIdx;
                if (!adj[v * V + w]) continue;               // not a neighbour
                if (!seen[w]) {
                    ++cur.childCnt;
                    par[w] = v;
                    stk[sp++] = { w, v, 0, 0 };
                    ++cur.nxtIdx;                               // advance after return
                    goDeeper = true;
                    break;
                } else if (w != par[v]) {
                    // low[v] = min(low[v], disc[w])
                    low[v] = (low[v] < disc[w] ? low[v] : disc[w]);
                }
            }

            if (goDeeper) continue;                           // process child

            // all neighbours processed, pop
            --sp;

            if (cur.parent != -1) {
                // low[parent] = min(low[parent], low[v])
                low[cur.parent] = (low[cur.parent] < low[v] ? low[cur.parent] : low[v]);
                // articulation test for non‑root
                if (low[v] >= disc[cur.parent]) art[cur.parent] = 1;
            } else {
                // root articulation test
                if (cur.childCnt > 1) art[v] = 1;
            }
        }
        delete[] stk;
    }

    // output articulation points
    std::cout << "Articulation points:" << std::endl;
    for (int i = 0; i < V; ++i) {
        if (art[i]) std::cout << i << ' ';
    }
    std::cout << std::endl;

    // clean up
    delete[] adj;
    delete[] disc;
    delete[] low;
    delete[] par;
    delete[] seen;
    delete[] art;

    return 0;
}
