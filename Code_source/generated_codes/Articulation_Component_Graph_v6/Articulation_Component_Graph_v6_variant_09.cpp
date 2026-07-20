#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */
class ArticulationSolver {
public:
    int V;               
    int Emax;            
    int* head;           
    int* nxt;            
    int* to;             
    int edgePtr;         

    int* dsc;            
    int* low;            
    int* par;            
    int* isArt;          
    int timer;           

    ArticulationSolver(int verts, int edges) {
        V = verts;
        Emax = edges * 2 + 5;                 
        head = new int[V];
        nxt  = new int[Emax];
        to   = new int[Emax];
        dsc  = new int[V];
        low  = new int[V];
        par  = new int[V];
        isArt = new int[V];
        for (int i = 0; i < V; ++i) head[i] = -1;
        edgePtr = 0;
        timer = 0;
    }

    ~ArticulationSolver() {
        delete[] head;
        delete[] nxt;
        delete[] to;
        delete[] dsc;
        delete[] low;
        delete[] par;
        delete[] isArt;
    }

    void addEdge(int a, int b) {
        to[edgePtr] = b;
        nxt[edgePtr] = head[a];
        head[a] = edgePtr;
        ++edgePtr;
        to[edgePtr] = a;
        nxt[edgePtr] = head[b];
        head[b] = edgePtr;
        ++edgePtr;
    }

    void runDFS(int src) {
        dsc[src] = low[src] = ++timer;
        isArt[src] = 0;
        int childCnt = 0;
        int e = head[src];
        while (e != -1) {
            int v = to[e];
            if (dsc[v] == 0) {
                ++childCnt;
                par[v] = src;
                runDFS(v);
                if (low[v] < low[src]) low[src] = low[v];
                if (par[src] != -1 && low[v] >= dsc[src]) isArt[src] = 1;
            } else if (v != par[src]) {
                if (dsc[v] < low[src]) low[src] = dsc[v];
            }
            e = nxt[e];
            if (e == -1) break;
            v = to[e];
            if (dsc[v] == 0) {
                ++childCnt;
                par[v] = src;
                runDFS(v);
                if (low[v] < low[src]) low[src] = low[v];
                if (par[src] != -1 && low[v] >= dsc[src]) isArt[src] = 1;
            } else if (v != par[src]) {
                if (dsc[v] < low[src]) low[src] = dsc[v];
            }
            e = nxt[e];
        }
        if (par[src] == -1 && childCnt > 1) isArt[src] = 1;
    }

    void findArticulationPoints() {
        for (int i = 0; i < V; ++i) {
            dsc[i] = 0;
            low[i] = 0;
            par[i] = -1;
            isArt[i] = 0;
        }
        timer = 0;
        for (int i = 0; i < V; ++i) {
            if (dsc[i] == 0) runDFS(i);
        }
    }

    void printResult() {
        std::cout << "Articulation points: ";
        int i = 0;
        while (i + 1 < V) {
            if (isArt[i])   std::cout << i << ' ';
            if (isArt[i+1]) std::cout << i+1 << ' ';
            i += 2;
        }
        if (i < V && isArt[i]) std::cout << i << ' ';
        std::cout << std::endl;
    }
};

int main() {
    int verts = 8;
    int undEdges = 9;
    ArticulationSolver solver(verts, undEdges);
    // deterministic pseudo‑random edge set
    solver.addEdge(0,3);
    solver.addEdge(0,5);
    solver.addEdge(1,2);
    solver.addEdge(1,6);
    solver.addEdge(2,4);
    solver.addEdge(3,5);
    solver.addEdge(4,6);
    solver.addEdge(5,7);
    solver.addEdge(2,7);
    solver.findArticulationPoints();
    solver.printResult();
    return 0;
}
