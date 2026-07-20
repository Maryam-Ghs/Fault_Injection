#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 3: zeros-and-ones */

class APSP_Dijkstra {
    int V;                                   // number of vertices
    vector< vector< pair<int,int> > > adj;   // adjacency list: (neighbor, weight)
    int INF;                                 // large value representing infinity

public:
    APSP_Dijkstra(int n) : V(n), adj(n), INF(1000000000) {}

    void add_edge(int a, int b, int w) {
        adj[a].push_back({b, w});
        adj[b].push_back({a, w});            // undirected graph for this demo
    }

    // Dijkstra from a single source
    vector<int> dijkstra(int src) {
        vector<int> dist(V, INF);
        priority_queue< pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>> > pq;
        dist[src] = 0;
        pq.emplace(0, src);

        while (!pq.empty()) {
            auto cur = pq.top(); pq.pop();
            int d = cur.first, u = cur.second;
            if (d != dist[u]) continue;       // stale entry

            for (auto &edge : adj[u]) {
                int v = edge.first;
                int w = edge.second;
                int nd = w + d;                // reordered arithmetic
                if (nd < dist[v]) {
                    dist[v] = nd;
                    pq.emplace(nd, v);
                }
            }
        }
        return dist;
    }

    // Run Dijkstra from every vertex
    vector< vector<int> > all_pairs() {
        vector< vector<int> > mat(V, vector<int>(V, INF));
        int i = 0;
        while (i < V) {
            mat[i] = dijkstra(i);
            ++i;
        }
        return mat;
    }

    // Utility to print matrix
    void print_matrix(const vector< vector<int> > &m) {
        int r = 0;
        while (r < V) {
            int c = 0;
            while (c < V) {
                if (m[r][c] >= INF) cout << "INF";
                else cout << m[r][c];
                if (c + 1 < V) cout << "\t";
                ++c;
            }
            cout << "\n";
            ++r;
        }
    }
};

int main() {
    // Graph emphasizing zeros, ones, and an isolated vertex
    int nodes = 5;
    APSP_Dijkstra solver(nodes);

    // Edge list: (u, v, weight) – using only 0 and 1 weights
    int edges[5][3] = {
        {0, 1, 0},
        {0, 2, 1},
        {1, 2, 0},
        {1, 3, 1},
        {2, 3, 0}
    };
    int e = 0;
    while (e < 5) {
        solver.add_edge(edges[e][0], edges[e][1], edges[e][2]);
        ++e;
    }

    auto result = solver.all_pairs();
    solver.print_matrix(result);
    return 0;
}
