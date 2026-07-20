#include <bits/stdc++.h>
using namespace std;

#define MAXN 10          // maximum number of variables
#define MAXM 20          // maximum number of clauses
#define MAXV (2*MAXN)    // vertices in implication graph
#define MAXE (4*MAXM)    // each clause adds 2 edges, each stored forward and reverse

/* LLM input variant 8: sparse-skewed */
int main() {
    /* ---------- deterministic sparse instance generation ---------- */
    int n = 10;                         // number of variables (choose any 5‑10)
    int m = 3;                         // number of clauses   (choose any 10‑20, here few to be sparse)

    // adjacency structures (forward graph)
    int head[MAXV];
    int to[MAXE];
    int nxt[MAXE];
    // adjacency structures (reverse graph)
    int rhead[MAXV];
    int rto[MAXE];
    int rnxt[MAXE];

    // initialise heads
    int i;
    for (i = 0; i < MAXV; ++i) {
        head[i] = -1;
        rhead[i] = -1;
    }

    int ecnt = 0;      // edge counter for forward graph
    int recnt = 0;     // edge counter for reverse graph

    // helper: map literal to vertex index
    // literal >0  => x_i   (true)   => 2*(i-1)
    // literal <0  => ¬x_i (false)  => 2*(|i|-1)+1
    auto litToIdx = [&](int lit)->int {
        int var = (lit > 0) ? lit : -lit;   // 1‑based variable number
        int base = 2 * (var - 1);
        return (lit > 0) ? base : base + 1;
    };

    // helper: add directed edge u -> v (and its reverse)
    auto addEdge = [&](int u, int v) {
        to[ecnt] = v;
        nxt[ecnt] = head[u];
        head[u] = ecnt;
        ++ecnt;

        rto[recnt] = u;
        rnxt[recnt] = rhead[v];
        rhead[v] = recnt;
        ++recnt;
    };

    // deterministic sparse clauses, heavily clustered on variables 1‑3, others isolated
    int clauses[3][2] = {
        { 1, -2 },   // (x1 ∨ ¬x2)
        { 3, 3 },    // (x3 ∨ x3) duplicate literal
        { 1, 1 }     // (x1 ∨ x1) duplicate literal
    };

    for (i = 0; i < m; ++i) {
        int aLit = clauses[i][0];
        int bLit = clauses[i][1];

        int aIdx = litToIdx(aLit);
        int bIdx = litToIdx(bLit);
        int naIdx = aIdx ^ 1;   // ¬a
        int nbIdx = bIdx ^ 1;   // ¬b

        // (a ∨ b)  ⇒  (¬a → b)  and  (¬b → a)
        addEdge(naIdx, bIdx);
        addEdge(nbIdx, aIdx);
    }

    int V = 2 * n;    // total vertices

    /* ---------- Kosaraju first pass (order) ---------- */
    int visited[MAXV];
    int order[MAXV];
    int ordCnt = 0;
    for (i = 0; i < V; ++i) visited[i] = 0;

    int stack[MAXV];
    int iterPos[MAXV];   // remembers next edge to explore for each vertex

    int v;
    for (v = 0; v < V; ++v) {
        if (visited[v]) continue;

        // start new DFS from v
        int sp = 0;
        stack[sp++] = v;
        iterPos[v] = head[v];

        while (sp > 0) {
            int cur = stack[sp - 1];
            if (!visited[cur]) visited[cur] = 1;

            int e = iterPos[cur];
            // skip already visited neighbours
            while (e != -1 && visited[to[e]]) e = nxt[e];

            if (e != -1) {
                // go deeper
                iterPos[cur] = nxt[e];
                int nb = to[e];
                if (!visited[nb]) {
                    stack[sp++] = nb;
                    iterPos[nb] = head[nb];
                }
            } else {
                // finished this vertex
                sp--;
                order[ordCnt++] = cur;
            }
        }
    }

    /* ---------- Kosaraju second pass (components) ---------- */
    for (i = 0; i < V; ++i) visited[i] = 0;
    int comp[MAXV];
    int compCnt = 0;

    for (i = ordCnt - 1; i >= 0; --i) {
        int start = order[i];
        if (visited[start]) continue;

        // new component
        int sp = 0;
        stack[sp++] = start;
        visited[start] = 1;
        comp[start] = compCnt;

        while (sp > 0) {
            int cur = stack[--sp];
            int e = rhead[cur];
            while (e != -1) {
                int nb = rto[e];
                if (!visited[nb]) {
                    visited[nb] = 1;
                    comp[nb] = compCnt;
                    stack[sp++] = nb;
                }
                e = rnxt[e];
            }
        }
        ++compCnt;
    }

    /* ---------- satisfiability check & assignment ---------- */
    int assignment[MAXN];
    int satisfiable = 1;
    for (i = 0; i < n; ++i) {
        if (comp[2*i] == comp[2*i + 1]) {
            satisfiable = 0;
            break;
        }
        // component with larger number appears earlier in topological order
        assignment[i] = (comp[2*i] > comp[2*i + 1]) ? 1 : 0;
    }

    /* ---------- output ---------- */
    if (satisfiable) {
        printf("SATISFIABLE\n");
        printf("Variable assignment (1 = true, 0 = false):\n");
        for (i = 0; i < n; ++i) {
            printf("x%d = %d\n", i + 1, assignment[i]);
        }
    } else {
        printf("UNSATISFIABLE\n");
    }

    return 0;
}
