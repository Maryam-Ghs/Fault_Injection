// Version #9 - 2-SAT Implication Graph (heap‑based, class‑oriented, modular)
/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

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
// Helper: deterministic pseudo‑random generator (LCG)
static unsigned int lcg_state = 0x9a5b3c1d;
static unsigned int lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// Helper: generate a deterministic 2‑SAT instance (medium size)
void generateRandomInstance(int& varCnt, int& clauseCnt,
                            int*& aVar, int*& aSign,
                            int*& bVar, int*& bSign) {
    // Deterministic but varied values
    varCnt = 13 + (lcg_next() % 8);          // 13 … 20 variables
    clauseCnt = 35 + (lcg_next() % 11);     // 35 … 45 clauses

    aVar = new int[clauseCnt];
    aSign = new int[clauseCnt];
    bVar = new int[clauseCnt];
    bSign = new int[clauseCnt];

    for (int i = 0; i < clauseCnt; ++i) {
        aVar[i] = lcg_next() % varCnt;
        aSign[i] = (lcg_next() & 1);   // 0 = negative, 1 = positive
        bVar[i] = lcg_next() % varCnt;
        bSign[i] = (lcg_next() & 1);
    }
}

// ------------------------------------------------------------------
int main() {
    int vCnt, cCnt;
    int *lA, *sA, *lB, *sB;
    generateRandomInstance(vCnt, cCnt, lA, sA, lB, sB);

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
