/* LLM input variant 2: small-diverse */
// 2‑SAT Implication Graph – version #6
// ------------------------------------------------------------
// This program builds a random 2‑SAT instance, constructs the
// implication graph on the heap, runs Kosaraju’s algorithm and
// prints whether the formula is satisfiable together with one
// possible assignment.
// ------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// Helper structures for a simple adjacency list built on the heap
// ------------------------------------------------------------
struct Edge {
    int to;
    int nxt;
};

struct Graph {
    int *head;      // start of list for each vertex
    Edge *edges;    // pool of edges
    int eCnt;       // next free position in edges
    int vCnt;       // number of vertices

    // allocate all needed memory
    void init(int vertices, int maxEdges) {
        vCnt = vertices;
        head = new int[vertices];
        for (int i = 0; i < vertices; ++i) head[i] = -1;
        edges = new Edge[maxEdges];
        eCnt = 0;
    }

    // add directed edge u → v
    void add(int u, int v) {
        edges[eCnt].to = v;
        edges[eCnt].nxt = head[u];
        head[u] = eCnt++;
    }

    // free heap memory
    void release() {
        delete[] head;
        delete[] edges;
    }
};

// ------------------------------------------------------------
// Kosaraju helper data
// ------------------------------------------------------------
int *order;          // finishing order of first DFS
int *comp;           // component id of each vertex
int *used;           // visited flag
int orderPos;        // next free slot in order array
int compCnt;         // number of found components

// ------------------------------------------------------------
// First pass – fill order array (iterative version)
// ------------------------------------------------------------
void dfs1(Graph &g, int start) {
    int *stack = new int[g.vCnt];
    int *it    = new int[g.vCnt];
    int sp = 0;

    stack[sp] = start;
    it[sp]   = g.head[start];
    used[start] = 1;
    ++sp;

    while (sp) {
        int v = stack[sp - 1];
        int &e = it[sp - 1];

        if (e != -1) {
            int to = g.edges[e].to;
            e = g.edges[e].nxt;
            if (!used[to]) {
                used[to] = 1;
                stack[sp] = to;
                it[sp]    = g.head[to];
                ++sp;
            }
        } else {
            order[--orderPos] = v;
            --sp;
        }
    }

    delete[] stack;
    delete[] it;
}

// ------------------------------------------------------------
// Second pass – assign component ids (iterative)
// ------------------------------------------------------------
void dfs2(Graph &gr, int start, int id) {
    int *stack = new int[gr.vCnt];
    int sp = 0;

    stack[sp++] = start;
    comp[start] = id;

    while (sp) {
        int v = stack[--sp];
        for (int e = gr.head[v]; e != -1; e = gr.edges[e].nxt) {
            int to = gr.edges[e].to;
            if (comp[to] == -1) {
                comp[to] = id;
                stack[sp++] = to;
            }
        }
    }

    delete[] stack;
}

// ------------------------------------------------------------
// Build a deterministic small‑diverse 2‑SAT instance
// ------------------------------------------------------------
void generateInstance(int &varCnt, int &clauseCnt,
                      int *&litA, int *&litB) {
    // deterministic small instance
    varCnt    = 5;                 // number of boolean variables
    clauseCnt = 8;                 // number of clauses (a ∨ b)

    litA = new int[clauseCnt];
    litB = new int[clauseCnt];

    // literals are encoded as 2*var + polarity (0 = false, 1 = true)
    // clause list:
    // (¬x1 ∨  x2)   -> 0 , 3
    // ( x3 ∨ ¬x4)    -> 5 , 6
    // (¬x5 ∨  x1)    -> 8 , 1
    // (¬x2 ∨ ¬x3)    -> 2 , 4
    // ( x4 ∨  x5)    -> 7 , 9
    // (¬x1 ∨  x3)    -> 0 , 5
    // ( x2 ∨ ¬x5)    -> 3 , 8
    // (¬x4 ∨  x1)    -> 6 , 1

    int aVals[8] = {0, 5, 8, 2, 7, 0, 3, 6};
    int bVals[8] = {3, 6, 1, 4, 9, 5, 8, 1};

    for (int i = 0; i < clauseCnt; ++i) {
        litA[i] = aVals[i];
        litB[i] = bVals[i];
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    // --------------------------------------------------------
    // 1. generate a deterministic small‑diverse 2‑SAT formula
    // --------------------------------------------------------
    int nVar, nClause;
    int *aLit, *bLit;
    generateInstance(nVar, nClause, aLit, bLit);

    // --------------------------------------------------------
    // 2. build implication graph and its transpose
    // --------------------------------------------------------
    int vertices = 2 * nVar;                      // each var has true/false node
    int maxEdges = 2 * nClause * 2;               // two edges per clause, each stored twice (orig + rev)

    Graph g, rg;
    g.init(vertices, maxEdges);
    rg.init(vertices, maxEdges);

    for (int i = 0; i < nClause; ++i) {
        int u = aLit[i];
        int v = bLit[i];
        int nu = u ^ 1;        // negation of u
        int nv = v ^ 1;        // negation of v

        // (¬u → v) and (¬v → u)
        g.add(nu, v);
        rg.add(v, nu);
        g.add(nv, u);
        rg.add(u, nv);
    }

    // --------------------------------------------------------
    // 3. Kosaraju – first pass
    // --------------------------------------------------------
    order = new int[vertices];
    used  = new int[vertices];
    for (int i = 0; i < vertices; ++i) used[i] = 0;
    orderPos = vertices;

    for (int i = 0; i < vertices; ++i) {
        if (!used[i]) dfs1(g, i);
    }

    // --------------------------------------------------------
    // 4. Kosaraju – second pass
    // --------------------------------------------------------
    comp = new int[vertices];
    for (int i = 0; i < vertices; ++i) comp[i] = -1;
    compCnt = 0;

    for (int idx = 0; idx < vertices; ++idx) {
        int v = order[idx];
        if (comp[v] == -1) {
            dfs2(rg, v, compCnt++);
        }
    }

    // --------------------------------------------------------
    // 5. check satisfiability and produce one assignment
    // --------------------------------------------------------
    int *assign = new int[nVar];
    bool ok = true;
    for (int i = 0; i < nVar; ++i) {
        if (comp[2 * i] == comp[2 * i + 1]) {
            ok = false;
            break;
        }
        // component with larger id appears earlier in topological order
        assign[i] = comp[2 * i] > comp[2 * i + 1];
    }

    // --------------------------------------------------------
    // 6. output
    // --------------------------------------------------------
    if (ok) {
        printf("SATISFIABLE\n");
        for (int i = 0; i < nVar; ++i) {
            printf("x%d = %d\n", i + 1, assign[i]);
        }
    } else {
        printf("UNSATISFIABLE\n");
    }

    // --------------------------------------------------------
    // 7. clean up heap memory
    // --------------------------------------------------------
    delete[] aLit;
    delete[] bLit;
    delete[] order;
    delete[] used;
    delete[] comp;
    delete[] assign;
    g.release();
    rg.release();

    return 0;
}
