/* LLM input variant 8: sparse-skewed */
// Version #7 - 2-SAT Implication Graph Implementation
// --------------------------------------------------

#include <bits/stdc++.h>
using namespace std;

// --------------------------------------------------
// Helper to map a literal to a node index
// --------------------------------------------------
int litToNode(int var, int sign) {
    // sign: 1 = true, 0 = false
    // node: 2*var + sign
    return 2 * var + sign;
}

// --------------------------------------------------
// Class based 2-SAT solver using Kosaraju SCC
// --------------------------------------------------
class TwoSatSolver {
public:
    int varCount;                     // number of boolean variables
    vector< vector<int> > graph;      // implication graph
    vector< vector<int> > revGraph;   // reversed graph
    vector<int> order;                // finishing order
    vector<int> comp;                 // component id per node
    vector<int> assign;               // final assignment

    // --------------------------------------------------
    // Constructor
    // --------------------------------------------------
    TwoSatSolver(int n) {
        varCount = n;
        graph.resize(2 * n);
        revGraph.resize(2 * n);
    }

    // --------------------------------------------------
    // Add implication (if a then b)
    // --------------------------------------------------
    void addImp(int aVar, int aSign, int bVar, int bSign) {
        int from = litToNode(aVar, aSign);
        int to   = litToNode(bVar, bSign);
        graph[from].push_back(to);
        revGraph[to].push_back(from);
    }

    // --------------------------------------------------
    // Add clause (x ∨ y)
    // --------------------------------------------------
    void addOr(int xVar, int xSign, int yVar, int ySign) {
        // ¬x → y  and  ¬y → x
        addImp(xVar, 1 - xSign, yVar, ySign);
        addImp(yVar, 1 - ySign, xVar, xSign);
    }

    // --------------------------------------------------
    // Depth‑first search for order (first pass)
    // --------------------------------------------------
    void dfs1(int v, vector<int>& used) {
        used[v] = 1;
        int i = 0;
        while (i < (int)graph[v].size()) {
            int to = graph[v][i];
            i = i + 1;                     // reordered increment
            if (used[to] == 0) {
                dfs1(to, used);
            }
        }
        order.push_back(v);
    }

    // --------------------------------------------------
    // Depth‑first search for components (second pass)
    // --------------------------------------------------
    void dfs2(int v, int cl) {
        comp[v] = cl;
        int i = 0;
        while (i < (int)revGraph[v].size()) {
            int to = revGraph[v][i];
            i = i + 1;
            if (comp[to] == -1) {
                dfs2(to, cl);
            }
        }
    }

    // --------------------------------------------------
    // Solve the 2‑SAT instance
    // --------------------------------------------------
    bool solve() {
        int nodes = 2 * varCount;
        vector<int> used(nodes, 0);
        order.clear();
        // first pass
        int v = 0;
        while (v < nodes) {
            if (used[v] == 0) {
                dfs1(v, used);
            }
            v = v + 1;
        }

        // second pass
        comp.assign(nodes, -1);
        int j = (int)order.size() - 1;
        while (j >= 0) {
            int vtx = order[j];
            if (comp[vtx] == -1) {
                dfs2(vtx, j);
            }
            j = j - 1;
        }

        // check contradictions and build assignment
        assign.assign(varCount, 0);
        int i = 0;
        while (i < varCount) {
            if (comp[2 * i] == comp[2 * i + 1]) {
                return false;               // unsatisfiable
            }
            // variable true if its component id is larger (topological order)
            assign[i] = (comp[2 * i] < comp[2 * i + 1]) ? 1 : 0;
            i = i + 1;
        }
        return true;
    }

    // --------------------------------------------------
    // Print the satisfying assignment
    // --------------------------------------------------
    void printResult() {
        if (solve()) {
            cout << "SATISFIABLE\n";
            int i = 0;
            while (i < varCount) {
                cout << "Var" << i << " = " << assign[i] << "\n";
                i = i + 1;
            }
        } else {
            cout << "UNSATISFIABLE\n";
        }
    }
};

// --------------------------------------------------
// Main entry point
// --------------------------------------------------
int main() {
    // Deterministic sparse-skewed instance
    const int nVars = 10;                 // total variables
    const int nClauses = 30;              // many clauses, but concentrated on few vars

    TwoSatSolver solver(nVars);

    cout << "Generated 2‑SAT instance (sparse-skewed):\n";
    cout << "Variables: " << nVars << "\n";
    cout << "Clauses:   " << nClauses << "\n";

    // First 20 clauses involve only variables 0 and 1 (highly clustered)
    for (int i = 0; i < 20; ++i) {
        int signA = (i % 2 == 0) ? 1 : 0;   // alternating signs
        int signB = (i % 3 == 0) ? 0 : 1;
        solver.addOr(0, signA, 1, signB);
        cout << "(" << (signA ? "" : "~") << "x0 ∨ "
             << (signB ? "" : "~") << "x1)\n";
    }

    // Remaining 10 clauses each touch a distinct higher-index variable once,
    // leaving most variables isolated or lightly used.
    for (int i = 0; i < 10; ++i) {
        int var = 2 + i;                  // variables 2..11 (but we have only up to 9)
        if (var >= nVars) var = nVars - 1; // clamp to existing range
        int signA = (i % 2);
        int signB = ((i + 1) % 2);
        solver.addOr(var, signA, var, signB); // clause (x_var ∨ ¬x_var) always true, keeps sparsity
        cout << "(" << (signA ? "" : "~") << "x" << var << " ∨ "
             << (signB ? "" : "~") << "x" << var << ")\n";
    }

    cout << "\nResult:\n";
    solver.printResult();

    return 0;
}
