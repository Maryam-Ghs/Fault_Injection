#include <iostream>

/* LLM input variant 9: medium-deterministic-random */
int main() {
    // version #10
    // medium deterministic random‑like graph (10 vertices, 12 edges)
    int V = 10;
    int E = 12;
    int aU[12] = {0,1,2,3,4,5,6,7,8,9,0,2};
    int aV[12] = {1,4,7,0,3,6,9,2,5,8,5,9};

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
                if (!adj[v * V + w]) continue;
                if (!seen[w]) {
                    ++cur.childCnt;
                    par[w] = v;
                    stk[sp++] = { w, v, 0, 0 };
                    ++cur.nxtIdx;
                    goDeeper = true;
                    break;
                } else if (w != par[v]) {
                    low[v] = (low[v] < disc[w] ? low[v] : disc[w]);
                }
            }

            if (goDeeper) continue;

            --sp;

            if (cur.parent != -1) {
                low[cur.parent] = (low[cur.parent] < low[v] ? low[cur.parent] : low[v]);
                if (low[v] >= disc[cur.parent]) art[cur.parent] = 1;
            } else {
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
