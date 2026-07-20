/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cstdio>

int main() {
    // ---------- generate a heavy edge‑case instance ----------
    int vN = 8;                     // number of variables
    int cM = 12;                    // number of clauses
    // literals are encoded as: 2*idx   ->  x_i (true)
    //                          2*idx+1 -> ¬x_i (false)
    int litA[12] = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7};
    int litB[12] = {1, 3, 5, 7, 9, 11, 13, 15, 0, 2, 4, 6};
    // The above clauses include a variety of forced assignments,
    // mixed literals, and tautologies across eight variables.

    // ---------- build implication graph (heap‑allocated) ----------
    int totalEdges = cM * 2;                      // each clause gives two implications
    int *toFwd = new int[totalEdges];
    int *nxtFwd = new int[totalEdges];
    int *headFwd = new int[2 * vN];
    int *toRev = new int[totalEdges];
    int *nxtRev = new int[totalEdges];
    int *headRev = new int[2 * vN];
    int eIdx = 0;                                 // edge index counter

    // initialise heads
    int i = 0;
    while (i < 2 * vN) {
        headFwd[i] = -1;
        headRev[i] = -1;
        i = i + 1;
    }

    // add edges, manual unrolling of two per clause
    i = 0;
    while (i < cM) {
        // first implication: ¬a → b
        int a = litA[i];
        int b = litB[i];
        int notA = a ^ 1;                         // flip last bit
        // store forward edge
        toFwd[eIdx] = b;
        nxtFwd[eIdx] = headFwd[notA];
        headFwd[notA] = eIdx;
        // store reverse edge
        toRev[eIdx] = notA;
        nxtRev[eIdx] = headRev[b];
        headRev[b] = eIdx;
        eIdx = eIdx + 1;

        // second implication: ¬b → a
        int notB = b ^ 1;
        // store forward edge
        toFwd[eIdx] = a;
        nxtFwd[eIdx] = headFwd[notB];
        headFwd[notB] = eIdx;
        // store reverse edge
        toRev[eIdx] = notB;
        nxtRev[eIdx] = headRev[a];
        headRev[a] = eIdx;
        eIdx = eIdx + 1;

        i = i + 1;
    }

    // ---------- Kosaraju's algorithm (all inside main) ----------
    int nodeCnt = 2 * vN;
    bool *vis = new bool[nodeCnt];
    int *order = new int[nodeCnt];
    int ordPos = 0;

    // initialise visited
    i = 0;
    while (i < nodeCnt) {
        vis[i] = false;
        i = i + 1;
    }

    // first DFS – compute finishing order
    i = 0;
    while (i < nodeCnt) {
        if (!vis[i]) {
            // iterative stack
            int *stk = new int[nodeCnt];
            int *ptr = new int[nodeCnt];
            int sp = 0;
            stk[sp] = i;
            ptr[sp] = headFwd[i];
            sp = sp + 1;
            vis[i] = true;

            while (sp) {
                int cur = stk[sp - 1];
                int e = ptr[sp - 1];
                if (e != -1) {
                    // advance iterator
                    ptr[sp - 1] = nxtFwd[e];
                    int nxtNode = toFwd[e];
                    if (!vis[nxtNode]) {
                        stk[sp] = nxtNode;
                        ptr[sp] = headFwd[nxtNode];
                        sp = sp + 1;
                        vis[nxtNode] = true;
                    }
                } else {
                    // finish node
                    sp = sp - 1;
                    order[ordPos] = cur;
                    ordPos = ordPos + 1;
                }
            }
            delete [] stk;
            delete [] ptr;
        }
        i = i + 1;
    }

    // second DFS – assign components
    int *comp = new int[nodeCnt];
    i = 0;
    while (i < nodeCnt) {
        comp[i] = -1;
        i = i + 1;
    }
    int compCnt = 0;

    // process vertices in reverse finishing order
    i = ordPos - 1;
    while (i >= 0) {
        int v = order[i];
        if (comp[v] == -1) {
            // iterative stack for reverse graph
            int *stk = new int[nodeCnt];
            int sp = 0;
            stk[sp] = v;
            sp = sp + 1;
            comp[v] = compCnt;

            while (sp) {
                int cur = stk[sp - 1];
                sp = sp - 1;
                int e = headRev[cur];
                while (e != -1) {
                    int nxtNode = toRev[e];
                    if (comp[nxtNode] == -1) {
                        stk[sp] = nxtNode;
                        sp = sp + 1;
                        comp[nxtNode] = compCnt;
                    }
                    e = nxtRev[e];
                }
            }
            delete [] stk;
            compCnt = compCnt + 1;
        }
        i = i - 1;
    }

    // ---------- check satisfiability ----------
    bool satisfiable = true;
    i = 0;
    while (i < vN) {
        if (comp[2 * i] == comp[2 * i + 1]) {
            satisfiable = false;
        }
        i = i + 1;
    }

    if (!satisfiable) {
        std::printf("UNSATISFIABLE\n");
    } else {
        std::printf("SATISFIABLE\n");
        // compute a concrete assignment: variable true if its component number is larger
        i = 0;
        while (i < vN) {
            int val = (comp[2 * i] > comp[2 * i + 1]) ? 1 : 0;
            std::printf("x%d = %d\n", i + 1, val);
            i = i + 1;
        }
    }

    // ---------- clean up ----------
    delete [] toFwd;
    delete [] nxtFwd;
    delete [] headFwd;
    delete [] toRev;
    delete [] nxtRev;
    delete [] headRev;
    delete [] vis;
    delete [] order;
    delete [] comp;

    return 0;
}
