#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

class Graph {
    int V;                                   // number of vertices
    std::vector< std::vector<int> > adj;     // adjacency list

    void dfsArtic(int u, int& time,
                  std::vector<int>& disc,
                  std::vector<int>& low,
                  std::vector<int>& parent,
                  std::vector<int>& articulation) {
        int children = 0;
        disc[u] = low[u] = ++time;

        int i = 0;
        while (i < (int)adj[u].size()) {
            int v = adj[u][i];
            if (disc[v] == -1) {
                parent[v] = u;
                ++children;
                dfsArtic(v, time, disc, low, parent, articulation);

                // Update low value after returning from recursion
                if (low[v] < low[u])
                    low[u] = low[v];

                // Articulation point check
                if (parent[u] == -1 && children > 1)
                    articulation[u] = 1;
                if (parent[u] != -1 && low[v] >= disc[u])
                    articulation[u] = 1;
            }
            else if (v != parent[u]) {
                // Update low value for back edge
                if (disc[v] < low[u])
                    low[u] = disc[v];
            }
            ++i;
        }
    }

public:
    Graph(int nodes) : V(nodes) {
        adj.assign(V, std::vector<int>());
    }

    void addEdge(int a, int b) {
        adj[a].push_back(b);
        adj[b].push_back(a);
    }

    std::vector<int> getArticulationPoints() {
        std::vector<int> disc(V, -1);
        std::vector<int> low(V, -1);
        std::vector<int> parent(V, -1);
        std::vector<int> articulation(V, 0);
        int timer = 0;

        int start = 0;
        while (start < V) {
            if (disc[start] == -1)
                dfsArtic(start, timer, disc, low, parent, articulation);
            ++start;
        }

        std::vector<int> result;
        int idx = 0;
        while (idx < V) {
            if (articulation[idx] == 1)
                result.push_back(idx);
            ++idx;
        }
        return result;
    }
};

int main() {
    std::srand((unsigned)std::time(0));

    // ---- minimal graph with a single articulation point ----
    int vertices = 3;               // smallest non‑trivial graph
    Graph g(vertices);

    g.addEdge(0, 1);
    g.addEdge(1, 2);

    // ---- compute articulation points ----
    std::vector<int> arts = g.getArticulationPoints();

    // ---- output results ----
    std::cout << "Articulation points in the generated graph:\n";
    int i = 0;
    while (i < (int)arts.size()) {
        std::cout << arts[i] << ' ';
        ++i;
    }
    if (arts.empty())
        std::cout << "None";
    std::cout << std::endl;

    return 0;
}
