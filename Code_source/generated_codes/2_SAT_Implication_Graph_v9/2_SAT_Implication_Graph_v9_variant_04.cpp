/* LLM input variant 4: signed-extremes */
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
        // aSign: 1 = positive, 0 = negative, -1 = negative (treated same as 0)
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
        // sign == 1 -> positive literal, otherwise negative
        // treat any non‑1 sign (including 0 and -1) as negative
        int s = (sign == 1) ? 0 : 1;
        return (varIdx << 1) | s;
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
// Helper: generate a random 2‑SAT instance (including zero & negative signs)
void generateRandomInstance(int& varCnt, int& clauseCnt,
                            int*& aVar, int*& aSign,
                            int*& bVar, int*& bSign) {
    varCnt = 5 + rand() % 11;          // 5 … 15 variables
    clauseCnt = rand() % 6;           // 0 … 5 clauses (mix zero)

    // allocate clause arrays on the heap
    aVar = new int[clauseCnt];
    aSign = new int[clauseCnt];
    bVar = new int[clauseCnt];
    bSign = new int[clauseCnt];

    const int signChoices[3] = { -1, 0, 1 }; // negative, zero, positive signs

    for (int i = 0; i < clauseCnt; ++i) {
        // random literals
        aVar[i] = rand() % varCnt;
        aSign[i] = signChoices[rand() % 3];
        bVar[i] = rand() % varCnt;
        bSign[i] = signChoices[rand() % 3];
    }
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
