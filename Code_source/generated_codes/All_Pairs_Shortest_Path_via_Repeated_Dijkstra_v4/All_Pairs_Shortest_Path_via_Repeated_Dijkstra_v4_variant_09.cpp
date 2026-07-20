#include <iostream>
#include <vector>
#include <queue>

/* LLM input variant 9: medium-deterministic-random */

int INF_VAL = 1000000000;

// Helper: Dijkstra for a single source
void run_dijkstra(int start_node,
                  const std::vector<std::vector<int>>& adj_mat,
                  std::vector<int>& out_dist)
{
    int v_cnt = adj_mat.size();

    // initialize distances
    for (int i = 0; i < v_cnt; ++i)
        out_dist[i] = INF_VAL;
    out_dist[start_node] = 0;

    // min‑heap of (distance, vertex)
    std::priority_queue<
        std::pair<int,int>,
        std::vector<std::pair<int,int>>,
        std::greater<std::pair<int,int>>
    > heap;

    heap.push(std::make_pair(0, start_node));

    while (!heap.empty())
    {
        int cur_dist = heap.top().first;
        int cur_vert = heap.top().second;
        heap.pop();

        // skip outdated entry
        if (cur_dist != out_dist[cur_vert])
            continue;

        // explore neighbours
        for (int nxt = 0; nxt < v_cnt; ++nxt)
        {
            int edge_w = adj_mat[cur_vert][nxt];
            if (edge_w == INF_VAL)           // no edge
                continue;

            int temp_sum = cur_dist + edge_w; // split math
            if (temp_sum < out_dist[nxt])
            {
                out_dist[nxt] = temp_sum;
                heap.push(std::make_pair(temp_sum, nxt));
            }
        }
    }
}

// Helper: run Dijkstra from every vertex
void all_pairs_shortest(const std::vector<std::vector<int>>& adj_mat,
                        std::vector<std::vector<int>>& result)
{
    int n = adj_mat.size();
    result.assign(n, std::vector<int>(n, INF_VAL));

    // temporary distance vector reused for each source
    std::vector<int> work_dist(n, INF_VAL);

    for (int src = 0; src < n; ++src)
    {
        run_dijkstra(src, adj_mat, work_dist);
        for (int dst = 0; dst < n; ++dst)
            result[src][dst] = work_dist[dst];
    }
}

// version #4 – deterministic test graph, loop‑heavy, split temp vars
int main()
{
    // deterministic graph: 12 vertices, pseudo‑random weighted directed edges
    const int node_cnt = 12;
    std::vector<std::vector<int>> graph(node_cnt,
                                        std::vector<int>(node_cnt, INF_VAL));

    // self‑loops are zero
    for (int i = 0; i < node_cnt; ++i)
        graph[i][i] = 0;

    // generate edges deterministically:
    // add an edge (i -> j) if (i + j) % 3 == 0 and i != j
    // weight = ((i * 7 + j * 13) % 15) + 2  (ensures weight in [2,16])
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            if (i == j) continue;
            if ((i + j) % 3 == 0)
            {
                int w = ((i * 7 + j * 13) % 15) + 2;
                graph[i][j] = w;
            }
        }
    }

    // compute all‑pairs shortest paths
    std::vector<std::vector<int>> shortest;
    all_pairs_shortest(graph, shortest);

    // print result matrix
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            int out_val = shortest[i][j];
            if (out_val == INF_VAL)
                std::cout << "INF";
            else
                std::cout << out_val;
            if (j + 1 < node_cnt)
                std::cout << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}
