// LLM input variant 2: small-diverse
// Version #9 - 2-SAT Implication Graph (heap‑based, class‑oriented, modular)
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

class TwoSatSolver {
public:
    TwoSatSolver(int varCount) {
        nVar = varCount;
        nNode = varCount * 2;
        // adjacency lists on the heap
        g = new std::vector<int>[nNode];
        rg = new std::vector<int>[nNode];
        comp = new int[nNode];
        order = new int[nNode];
        assign = new int[varCount];
        used = new bool[nNode];
        for (int i = 0; i < nNode; ++i) used[i] = false;
        for (int i = 0; i < nNode; ++i) comp[i] = -1;
    }

    ~TwoSatSolver() {
        delete[] g;
        delete[] rg;
        delete[] comp;
        delete[] order;
        delete[] assign;
        delete[] used;
    }

    // literal encoding: x -> 2*x, ¬x -> 2*x+1
    void addClause(int aVar, int aSign, int bVar, int bSign) {
        // aSign: 1 = positive, 0 = negative
        // bSign: same
        int litA = encode(aVar, aSign);
        int litB = encode(bVar, bSign);
        int nLitA = litA ^ 1;
        int nLitB = litB ^ 1;

        // (A ∨ B) is equivalent to (¬A → B) and (¬B → A)
        addEdge(nLitA, litB);
        addEdge(nLitB, litA);
    }

    bool solve() {
        // first pass: order vertices
        int idx = 0;
        for (int i = 0; i < nNode; ++i) {
            if (!used[i]) dfs1(i, idx);
        }

        // second pass: assign components
        int curComp = 0;
        for (int i = nNode - 1; i >= 0; --i) {
            int v = order[i];
            if (comp[v] == -1) dfs2(v, curComp++);
        }

        // check for contradictions
        for (int i = 0; i < nVar; ++i) {
            if (comp[2 * i] == comp[2 * i + 1]) return false;
            assign[i] = (comp[2 * i] > comp[2 * i + 1]) ? 1 : 0;
        }
        return true;
    }

    void printResult() {
        if (!solve()) {
            std::cout << "UNSATISFIABLE\n";
            return;
        }
        std::cout << "SATISFIABLE\n";
        for (int i = 0; i < nVar; ++i) {
            std::cout << "Var" << i << " = " << assign[i] << "\n";
        }
    }

private:
    int nVar;          // number of variables
    int nNode;         // 2 * nVar
    std::vector<int>* g;   // forward graph
    std::vector<int>* rg;  // reverse graph
    int* comp;         // component id
    int* order;        // finishing order
    int* assign;       // final assignment
    bool* used;        // visited flag

    int encode(int varIdx, int sign) {
        // sign == 1 -> positive literal, else negative
        return (varIdx << 1) | (sign ^ 1);
    }

    void addEdge(int from, int to) {
        g[from].push_back(to);
        rg[to].push_back(from);
    }

    void dfs1(int v, int& pos) {
        used[v] = true;
        // explore forward edges
        for (size_t i = 0; i < g[v].size(); ++i) {
            int nxt = g[v][i];
            if (!used[nxt]) dfs1(nxt, pos);
        }
        order[pos++] = v;
    }

    void dfs2(int v, int label) {
        comp[v] = label;
        // explore reverse edges
        for (size_t i = 0; i < rg[v].size(); ++i) {
            int nxt = rg[v][i];
            if (comp[nxt] == -1) dfs2(nxt, label);
        }
    }
};

// ------------------------------------------------------------------
// Helper: generate a deterministic small 2‑SAT instance (diverse literals)
void generateDeterministicInstance(int& varCnt, int& clauseCnt,
                                   int*& aVar, int*& aSign,
                                   int*& bVar, int*& bSign) {
    varCnt = 5;      // 5 variables: 0..4
    clauseCnt = 7;   // 7 clauses

    aVar = new int[clauseCnt];
    aSign = new int[clauseCnt];
    bVar = new int[clauseCnt];
    bSign = new int[clauseCnt];

    // Manually crafted diverse clauses
    // Clause 0: (x0 ∨ ¬x1)
    aVar[0] = 0; aSign[0] = 1;
    bVar[0] = 1; bSign[0] = 0;
    // Clause 1: (¬x0 ∨ x2)
    aVar[1] = 0; aSign[1] = 0;
    bVar[1] = 2; bSign[1] = 1;
    // Clause 2: (x3 ∨ x4)
    aVar[2] = 3; aSign[2] = 1;
    bVar[2] = 4; bSign[2] = 1;
    // Clause 3: (¬x2 ∨ ¬x3)
    aVar[3] = 2; aSign[3] = 0;
    bVar[3] = 3; bSign[3] = 0;
    // Clause 4: (x1 ∨ x3)
    aVar[4] = 1; aSign[4] = 1;
    bVar[4] = 3; bSign[4] = 1;
    // Clause 5: (¬x4 ∨ x0)
    aVar[5] = 4; aSign[5] = 0;
    bVar[5] = 0; bSign[5] = 1;
    // Clause 6: (¬x1 ∨ ¬x4)
    aVar[6] = 1; aSign[6] = 0;
    bVar[6] = 4; bSign[6] = 0;
}

// ------------------------------------------------------------------
int main() {
    // No need for randomness in this variant
    int vCnt, cCnt;
    int *lA, *sA, *lB, *sB;
    generateDeterministicInstance(vCnt, cCnt, lA, sA, lB, sB);

    TwoSatSolver solver(vCnt);
    for (int i = 0; i < cCnt; ++i) {
        solver.addClause(lA[i], sA[i], lB[i], sB[i]);
    }

    std::cout << "Variables: " << vCnt << "\n";
    std::cout << "Clauses  : " << cCnt << "\n";
    solver.printResult();

    // free clause memory
    delete[] lA;
    delete[] sA;
    delete[] lB;
    delete[] sB;
    return 0;
}
