// LLM input variant 10: large-safe-stress
// 2‑SAT Implication Graph – version #3
// Edge‑case heavy internal test, manual loop unrolling, heap arrays, class‑based.

#include <iostream>
#include <cstdio>
#include <cstdlib>

// Simple dynamic adjacency list stored on the heap
struct EdgeList {
    int *to;      // destination vertices
    int  cnt;     // current number of edges
    int  cap;     // allocated capacity
};

// 2‑SAT solver class
class TwoSatSolver {
    int   varN;          // number of variables
    int   nodeN;         // 2 * varN
    EdgeList *g;         // implication graph
    EdgeList *rg;        // reverse graph
    int   *orderArr;     // order of vertices (first pass)
    int    orderPos;     // position in orderArr
    int   *compArr;      // component id for each vertex
    int   *stk;          // manual stack for DFS
    int    stkTop;       // stack pointer
    int   *assign;       // final assignment (0/1)

public:
    TwoSatSolver(int v) {
        varN   = v;
        nodeN  = v * 2;
        g      = new EdgeList[nodeN];
        rg     = new EdgeList[nodeN];
        int i = 0;
        while (i < nodeN) {
            g[i].to  = nullptr; g[i].cnt = 0; g[i].cap = 0;
            rg[i].to = nullptr; rg[i].cnt = 0; rg[i].cap = 0;
            i = i + 1;
        }
        orderArr = new int[nodeN];
        compArr  = new int[nodeN];
        stk      = new int[nodeN];
        assign   = new int[varN];
    }

    ~TwoSatSolver() {
        int i = 0;
        while (i < nodeN) {
            if (g[i].to)  delete [] g[i].to;
            if (rg[i].to) delete [] rg[i].to;
            i = i + 1;
        }
        delete [] g;
        delete [] rg;
        delete [] orderArr;
        delete [] compArr;
        delete [] stk;
        delete [] assign;
    }

    // literal encoding: x = 2*var + (val ? 1 : 0)
    // val = 1 → true, val = 0 → false
    int lit(int var, int val) {
        int res = var * 2 + val;
        return res;
    }

    // negation of a literal
    int neg(int l) {
        int res = l ^ 1;
        return res;
    }

    // ensure capacity for adjacency list
    void ensureCap(EdgeList &e) {
        if (e.cnt == e.cap) {
            int newCap = e.cap + 4;                 // small growth factor
            int *newArr = new int[newCap];
            int j = 0;
            while (j < e.cnt) {
                newArr[j] = e.to[j];
                j = j + 1;
            }
            if (e.to) delete [] e.to;
            e.to  = newArr;
            e.cap = newCap;
        }
    }

    // add directed edge u → v
    void addImp(int u, int v) {
        ensureCap(g[u]);
        g[u].to[g[u].cnt] = v;
        g[u].cnt = g[u].cnt + 1;

        ensureCap(rg[v]);
        rg[v].to[rg[v].cnt] = u;
        rg[v].cnt = rg[v].cnt + 1;
    }

    // add clause (a ∨ b)
    void addOr(int aVar, int aVal, int bVar, int bVal) {
        int aLit = lit(aVar, aVal);
        int bLit = lit(bVar, bVal);
        // ¬a → b
        addImp(neg(aLit), bLit);
        // ¬b → a
        addImp(neg(bLit), aLit);
    }

    // first DFS pass (fill orderArr)
    void dfs1(int v) {
        compArr[v] = -1;               // reuse compArr as visited marker
        int i = 0;
        while (i < g[v].cnt) {
            int to = g[v].to[i];
            if (compArr[to] == -2) {   // -2 means unvisited
                dfs1(to);
            }
            i = i + 1;
        }
        orderArr[orderPos] = v;
        orderPos = orderPos + 1;
    }

    // second DFS pass (assign components)
    void dfs2(int v, int label) {
        compArr[v] = label;
        // manual loop unrolling for neighbours of reverse graph
        int i = 0;
        int cnt = rg[v].cnt;
        while (i + 1 < cnt) {
            int to1 = rg[v].to[i];
            int to2 = rg[v].to[i + 1];
            if (compArr[to1] == -2) dfs2(to1, label);
            if (compArr[to2] == -2) dfs2(to2, label);
            i = i + 2;
        }
        if (i < cnt) {
            int to = rg[v].to[i];
            if (compArr[to] == -2) dfs2(to, label);
        }
    }

    // solve and fill assign[], return true if satisfiable
    bool solve() {
        // initialise visited markers
        int i = 0;
        while (i < nodeN) {
            compArr[i] = -2;          // -2 = unvisited
            i = i + 1;
        }
        orderPos = 0;
        // first pass
        i = 0;
        while (i < nodeN) {
            if (compArr[i] == -2) dfs1(i);
            i = i + 1;
        }
        // second pass
        int label = 0;
        i = nodeN - 1;
        while (i >= 0) {
            int v = orderArr[i];
            if (compArr[v] == -2) {
                dfs2(v, label);
                label = label + 1;
            }
            i = i - 1;
        }
        // check for contradictions
        i = 0;
        while (i < varN) {
            if (compArr[2*i] == compArr[2*i + 1]) return false;
            // variable is true if component of true literal > component of false literal
            assign[i] = compArr[2*i] < compArr[2*i + 1];
            i = i + 1;
        }
        return true;
    }

    // print result
    void printResult() {
        if (solve()) {
            std::printf("SAT\n");
            int i = 0;
            while (i < varN) {
                std::printf("x%d = %d\n", i, assign[i]);
                i = i + 1;
            }
        } else {
            std::printf("UNSAT\n");
        }
    }
};

int main() {
    // Edge‑case heavy internal test suite
    // 1) Empty formula (should be SAT)
    {
        TwoSatSolver solver0(0);
        solver0.printResult();                     // expect SAT (trivial)
    }

    // 2) Simple satisfiable instance
    {
        TwoSatSolver solver1(3);
        // (x0 ∨ x1) ∧ (¬x0 ∨ x2) ∧ (¬x2 ∨ ¬x1)
        solver1.addOr(0, 1, 1, 1);
        solver1.addOr(0, 0, 2, 1);
        solver1.addOr(2, 0, 1, 0);
        solver1.printResult();                     // expect SAT
    }

    // 3) Contradictory instance (UNSAT)
    {
        TwoSatSolver solver2(2);
        // (x0) ∧ (¬x0)
        solver2.addOr(0, 1, 0, 1);   // (x0 ∨ x0) → forces x0 true
        solver2.addOr(0, 0, 0, 0);   // (¬x0 ∨ ¬x0) → forces x0 false
        solver2.printResult();                     // expect UNSAT
    }

    // 4) Large edge‑case: many variables, many clauses, duplicate edges
    {
        int V = 2000;  // increased size for stress testing while staying safe
        TwoSatSolver solver3(V);
        int i = 0;
        while (i < V) {
            // each variable appears in a chain of implications
            int nxt = (i + 1) % V;
            // (xi ∨ x_{i+1})
            solver3.addOr(i, 1, nxt, 1);
            // (¬xi ∨ ¬x_{i+1})
            solver3.addOr(i, 0, nxt, 0);
            // duplicate clause to stress edge handling
            solver3.addOr(i, 1, nxt, 1);
            i = i + 1;
        }
        solver3.printResult();                     // should be SAT
    }

    return 0;
}
