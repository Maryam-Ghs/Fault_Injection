/* LLM input variant 3: zeros-and-ones */
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
// Helper: generate a deterministic 2‑SAT instance emphasizing zeros and ones
void generateRandomInstance(int& varCnt, int& clauseCnt,
                            int*& aVar, int*& aSign,
                            int*& bVar, int*& bSign) {
    // Fixed small instance with many zero/one values
    varCnt = 2;      // variables: 0 and 1
    clauseCnt = 4;   // four clauses

    aVar = new int[clauseCnt];
    aSign = new int[clauseCnt];
    bVar = new int[clauseCnt];
    bSign = new int[clauseCnt];

    // Clause 0: (x0 ∨ x0)          both positive, indices 0, signs 1
    aVar[0] = 0; aSign[0] = 1;
    bVar[0] = 0; bSign[0] = 1;

    // Clause 1: (¬x0 ∨ ¬x0)       both negative, signs 0
    aVar[1] = 0; aSign[1] = 0;
    bVar[1] = 0; bSign[1] = 0;

    // Clause 2: (x1 ∨ ¬x1)        mixed signs
    aVar[2] = 1; aSign[2] = 1;
    bVar[2] = 1; bSign[2] = 0;

    // Clause 3: (¬x1 ∨ x1)        opposite order, same literals
    aVar[3] = 1; aSign[3] = 0;
    bVar[3] = 1; bSign[3] = 1;
}

// ------------------------------------------------------------------
int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

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
