#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */

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

    // Duplicate‑heavy graph: repeating the same edges many times
    int n = 4;
    Graph g(n);

    g.addEdge(0, 1);
    g.addEdge(0, 1); // duplicate
    g.addEdge(1, 2);
    g.addEdge(1, 2); // duplicate
    g.addEdge(1, 2); // duplicate
    g.addEdge(2, 3);
    g.addEdge(0, 1); // another duplicate

    g.displayMatrix();
    g.findArticulationPoints();
    g.printArticulationPoints();

    return 0;
}
