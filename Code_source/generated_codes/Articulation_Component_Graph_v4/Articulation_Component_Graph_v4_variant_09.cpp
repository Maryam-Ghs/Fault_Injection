#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

class Graph {
public:
    int V;
    int** matrix;
    int* visitedTime;
    int* lowLink;
    bool* cutVertex;
    int* prev;
    int timer;

    Graph(int nodes) {
        V = nodes;
        matrix = new int*[V];
        int i = 0;
        while (i < V) {
            matrix[i] = new int[V];
            int j = 0;
            while (j < V) {
                matrix[i][j] = 0;
                ++j;
            }
            ++i;
        }
        visitedTime = new int[V];
        lowLink = new int[V];
        cutVertex = new bool[V];
        prev = new int[V];
        i = 0;
        while (i < V) {
            visitedTime[i] = -1;
            lowLink[i] = -1;
            cutVertex[i] = false;
            prev[i] = -1;
            ++i;
        }
        timer = 0;
    }

    ~Graph() {
        int i = 0;
        while (i < V) {
            delete[] matrix[i];
            ++i;
        }
        delete[] matrix;
        delete[] visitedTime;
        delete[] lowLink;
        delete[] cutVertex;
        delete[] prev;
    }

    void addEdge(int a, int b) {
        if (a >= 0 && a < V && b >= 0 && b < V) {
            matrix[a][b] = 1;
            matrix[b][a] = 1;
        }
    }

    void findArticulationPoints() {
        int v = 0;
        while (v < V) {
            if (visitedTime[v] == -1)
                dfs(v);
            ++v;
        }
    }

    void dfs(int src) {
        visitedTime[src] = timer;
        lowLink[src] = timer;
        ++timer;

        int children = 0;
        int u = 0;
        while (u < V) {
            if (matrix[src][u]) {
                if (visitedTime[u] == -1) {
                    prev[u] = src;
                    ++children;
                    dfs(u);
                    if (lowLink[u] < lowLink[src])
                        lowLink[src] = lowLink[u];
                    if (prev[src] != -1 && lowLink[u] >= visitedTime[src])
                        cutVertex[src] = true;
                } else if (u != prev[src]) {
                    if (visitedTime[u] < lowLink[src])
                        lowLink[src] = visitedTime[u];
                }
            }
            ++u;
        }

        if (prev[src] == -1 && children > 1)
            cutVertex[src] = true;
    }

    void printArticulationPoints() {
        std::cout << "Articulation points (0‑based indices): ";
        int i = 0;
        bool any = false;
        while (i < V) {
            if (cutVertex[i]) {
                std::cout << i << ' ';
                any = true;
            }
            ++i;
        }
        if (!any)
            std::cout << "none";
        std::cout << std::endl;
    }

    void displayMatrix() {
        std::cout << "Adjacency matrix:" << std::endl;
        int i = 0;
        while (i < V) {
            int j = 0;
            while (j < V) {
                std::cout << matrix[i][j] << ' ';
                ++j;
            }
            std::cout << std::endl;
            ++i;
        }
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    // Deterministic pseudo‑random pattern on 8 vertices
    int n = 8;
    Graph g(n);

    // Edge pattern: (i, (3*i + 1) % n)
    g.addEdge(0, 1);
    g.addEdge(1, 4);
    g.addEdge(2, 7);
    g.addEdge(3, 2);
    g.addEdge(4, 5);
    g.addEdge(5, 0);
    g.addEdge(6, 3);
    g.addEdge(7, 6);
    // Duplicates may arise naturally; addEdge safely ignores repeats

    g.displayMatrix();
    g.findArticulationPoints();
    g.printArticulationPoints();

    return 0;
}
