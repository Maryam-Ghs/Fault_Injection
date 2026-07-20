/ * LLM input variant 1: minimal-boundary * /
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

    void addClause(int aVar, int aSign, int bVar, int bSign) {
        int litA = encode(aVar, aSign);
        int litB = encode(bVar, bSign);
        int nLitA = litA ^ 1;
        int nLitB = litB ^ 1;
        addEdge(nLitA, litB);
        addEdge(nLitB, litA);
    }

    bool solve() {
        int idx = 0;
        for (int i = 0; i < nNode; ++i) {
            if (!used[i]) dfs1(i, idx);
        }
        int curComp = 0;
        for (int i = nNode - 1; i >= 0; --i) {
            int v = order[i];
            if (comp[v] == -1) dfs2(v, curComp++);
        }
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
    int nVar;
    int nNode;
    std::vector<int>* g;
    std::vector<int>* rg;
    int* comp;
    int* order;
    int* assign;
    bool* used;

    int encode(int varIdx, int sign) {
        return (varIdx << 1) | (sign ^ 1);
    }

    void addEdge(int from, int to) {
        g[from].push_back(to);
        rg[to].push_back(from);
    }

    void dfs1(int v, int& pos) {
        used[v] = true;
        for (size_t i = 0; i < g[v].size(); ++i) {
            int nxt = g[v][i];
            if (!used[nxt]) dfs1(nxt, pos);
        }
        order[pos++] = v;
    }

    void dfs2(int v, int label) {
        comp[v] = label;
        for (size_t i = 0; i < rg[v].size(); ++i) {
            int nxt = rg[v][i];
            if (comp[nxt] == -1) dfs2(nxt, label);
        }
    }
};

void generateMinimalInstance(int& varCnt, int& clauseCnt,
                             int*& aVar, int*& aSign,
                             int*& bVar, int*& bSign) {
    varCnt = 1;
    clauseCnt = 1;
    aVar = new int[clauseCnt];
    aSign = new int[clauseCnt];
    bVar = new int[clauseCnt];
    bSign = new int[clauseCnt];
    aVar[0] = 0;   // variable index 0
    aSign[0] = 1;  // positive literal
    bVar[0] = 0;   // same variable
    bSign[0] = 0;  // negative literal
}

int main() {
    int vCnt, cCnt;
    int *lA, *sA, *lB, *sB;
    generateMinimalInstance(vCnt, cCnt, lA, sA, lB, sB);

    TwoSatSolver solver(vCnt);
    for (int i = 0; i < cCnt; ++i) {
        solver.addClause(lA[i], sA[i], lB[i], sB[i]);
    }

    std::cout << "Variables: " << vCnt << "\n";
    std::cout << "Clauses  : " << cCnt << "\n";
    solver.printResult();

    delete[] lA;
    delete[] sA;
    delete[] lB;
    delete[] sB;
    return 0;
}
