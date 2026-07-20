#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 4: signed-extremes */

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
    // Modified graph with mixed signed edge weights (6 vertices)
    int nodes = 6;
    APSP_Dijkstra solver(nodes);

    // Edge list: (u, v, weight) includes negative, zero, and positive weights
    int edges[8][3] = {
        {0, 1, 0},
        {0, 2, 5},
        {1, 2, -3},
        {1, 3, 2},
        {2, 3, 0},
        {2, 4, 7},
        {3, 4, -2},
        {4, 5, 4}
    };
    int e = 0;
    while (e < 8) {
        solver.add_edge(edges[e][0], edges[e][1], edges[e][2]);
        ++e;
    }

    auto result = solver.all_pairs();
    solver.print_matrix(result);
    return 0;
}
