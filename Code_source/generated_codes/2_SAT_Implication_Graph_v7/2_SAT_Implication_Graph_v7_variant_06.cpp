/* LLM input variant 6: ordered-structured */
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
    // Deterministic, ordered, highly structured test case
    int nVars = 8;                 // fixed number of variables
    int nClauses = 16;             // fixed number of clauses

    TwoSatSolver solver(nVars);

    cout << "Generated 2‑SAT instance:\n";
    cout << "Variables: " << nVars << "\n";
    cout << "Clauses:   " << nClauses << "\n";

    int c = 0;
    while (c < nClauses) {
        // Structured pattern:
        // a cycles through variables, b is the next variable (mod nVars)
        // sign alternates to create a mix of positive and negative literals
        int a = c % nVars;
        int b = (c + 1) % nVars;
        int signA = (c % 2);          // 0,1,0,1,...
        int signB = ((c + 1) % 2);
        solver.addOr(a, signA, b, signB);
        cout << "(" << (signA ? "" : "~") << "x" << a << " ∨ "
             << (signB ? "" : "~") << "x" << b << ")\n";
        c = c + 1;
    }

    cout << "\nResult:\n";
    solver.printResult();

    return 0;
}
