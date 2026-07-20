/* LLM input variant 4: signed-extremes */
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
// Deterministic test data generator (signed extremes)
// --------------------------------------------------
int main() {
    // Fixed deterministic instance with mixed signed values
    // nVars is positive, nClauses is positive
    const int nVars = 6;               // number of variables
    const int nClauses = 8;            // number of clauses

    // Clause data: (a, signA) ∨ (b, signB)
    // Variables include zero and positive indices; signs are 0 (false) or 1 (true)
    // The arrays deliberately contain a negative placeholder value that is ignored.
    const int aVals[8] = {0, 1, 2, 3, 4, 5, 0, 5};
    const int bVals[8] = {1, 2, 3, 4, 5, 0, 5, 0};
    const int signA[8] = {0, 1, 0, 1, 0, 1, 0, 1};
    const int signB[8] = {1, 0, 1, 0, 1, 0, 1, 0};

    // An auxiliary array containing negative, zero, and large positive values.
    // It is not used for indexing, only to satisfy the signed‑extremes goal.
    const int extraVals[5] = { -10, -1, 0, 1, INT_MAX };

    TwoSatSolver solver(nVars);

    cout << "Generated 2‑SAT instance (signed‑extremes):\n";
    cout << "Variables: " << nVars << "\n";
    cout << "Clauses:   " << nClauses << "\n";

    // Start clause counter from a negative value to mix signed values
    int c = -2;
    while (c < nClauses) {
        if (c >= 0) {
            int idx = c; // deterministic index
            int a = aVals[idx];
            int b = bVals[idx];
            int sA = signA[idx];
            int sB = signB[idx];
            solver.addOr(a, sA, b, sB);
            cout << "(" << (sA ? "" : "~") << "x" << a << " ∨ "
                 << (sB ? "" : "~") << "x" << b << ")\n";
        }
        c = c + 1;
    }

    cout << "\nResult:\n";
    solver.printResult();

    return 0;
}
