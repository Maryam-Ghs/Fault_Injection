/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <iomanip>

#define MAXV 5                 // maximum number of vertices
#define INF  1000000000        // representation of infinity

/*--------------------------------------------------------------
   Generate a deterministic weighted directed graph.
   The graph matrix is stored in 'adj' (size: n x MAXV).
----------------------------------------------------------------*/
void build_graph(int n, int adj[][MAXV])
{
    // initialise with INF (no edge)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            adj[i][j] = (i == j) ? 0 : INF;

    // deterministic edges emphasizing zeros and ones
    // 0 -> 1 (1), 0 -> 2 (0)
    // 1 -> 2 (1), 1 -> 3 (0)
    // 2 -> 3 (1), 2 -> 4 (0)
    // 3 -> 4 (1)
    // 4 -> 0 (0)
    adj[0][1] = 1;  adj[0][2] = 0;
    adj[1][2] = 1;  adj[1][3] = 0;
    adj[2][3] = 1;  adj[2][4] = 0;
    adj[3][4] = 1;
    adj[4][0] = 0;
}

/*--------------------------------------------------------------
   Single‑source shortest paths using Dijkstra (no heap).
   'src' is the source vertex, 'adj' holds edge weights,
   and the result is written to 'dist_out'.
----------------------------------------------------------------*/
void run_dijkstra(int n, int src, int adj[][MAXV], int dist_out[])
{
    int visited[MAXV];
    // initialise distance and visited arrays
    for (int i = 0; i < n; ++i) {
        dist_out[i] = INF;
        visited[i] = 0;
    }
    dist_out[src] = 0;

    // main loop: repeat n times
    for (int iter = 0; iter < n; ++iter) {
        int best = -1;
        int best_val = INF;

        // find the unvisited vertex with smallest tentative distance
        for (int v = 0; v < n; ++v) {
            int cur_dist = dist_out[v];
            int is_unvisited = visited[v] == 0;
            if (is_unvisited && cur_dist < best_val) {
                best_val = cur_dist;
                best = v;
            }
        }

        // if no reachable vertex remains, break
        if (best == -1) break;

        visited[best] = 1;               // mark as processed

        // relax edges outgoing from 'best'
        for (int nxt = 0; nxt < n; ++nxt) {
            int edge_w = adj[best][nxt];
            int reachable = edge_w < INF;
            int not_visited = visited[nxt] == 0;
            if (reachable && not_visited) {
                int alt = dist_out[best] + edge_w;   // temporary variable
                if (alt < dist_out[nxt])
                    dist_out[nxt] = alt;             // update distance
            }
        }
    }
}

/*--------------------------------------------------------------
   Compute all‑pairs shortest paths by invoking Dijkstra from
   every source vertex. The result matrix 'all_dist' is filled.
----------------------------------------------------------------*/
void compute_all_pairs(int n, int adj[][MAXV], int all_dist[][MAXV])
{
    // temporary buffer for one‑source distances
    int temp_dist[MAXV];

    for (int s = 0; s < n; ++s) {
        // run Dijkstra for source 's'
        run_dijkstra(n, s, adj, temp_dist);

        // copy the result into the appropriate row
        for (int t = 0; t < n; ++t) {
            all_dist[s][t] = temp_dist[t];
        }
    }
}

/*--------------------------------------------------------------
   Print the distance matrix in a readable format.
----------------------------------------------------------------*/
void show_matrix(int n, int mat[][MAXV])
{
    std::cout << "All‑Pairs Shortest Path distances:\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (mat[i][j] >= INF)
                std::cout << std::setw(5) << "INF";
            else
                std::cout << std::setw(5) << mat[i][j];
        }
        std::cout << '\n';
    }
}

/*--------------------------------------------------------------
   Entry point.
----------------------------------------------------------------*/
int main()
{
    int vertex_cnt = 5;                 // deterministic size
    int adjacency[MAXV][MAXV];
    int shortest[MAXV][MAXV];

    build_graph(vertex_cnt, adjacency);
    compute_all_pairs(vertex_cnt, adjacency, shortest);
    show_matrix(vertex_cnt, shortest);

    return 0;
}
