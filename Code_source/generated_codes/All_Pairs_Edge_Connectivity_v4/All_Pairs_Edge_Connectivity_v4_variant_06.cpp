#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

/* LLM input variant 6: ordered-structured */

int main() {
    // ---------- deterministic structured graph generation ----------
    int vert_cnt = 9;                         // number of vertices
    int edge_cnt = vert_cnt * (vert_cnt-1) / 2;
    std::vector<std::vector<int>> base_cap( vert_cnt, std::vector<int>( vert_cnt, 0 ) );

    // fill upper triangle with a structured pattern: capacities = (i + j) % 15 + 1
    for (int i = 0; i < vert_cnt; ++i) {
        for (int j = i + 1; j < vert_cnt; ++j) {
            int cap_val = (i + j) % 15 + 1;
            base_cap[i][j] = cap_val;
            base_cap[j][i] = cap_val;
        }
    }

    // ---------- helper lambdas ----------
    // BFS that also records parent vertices
    auto bfs_path = [&](int src, int dst,
                        const std::vector<std::vector<int>>& residual,
                        std::vector<int>& pred) -> bool {
        std::vector<int> seen( vert_cnt, 0 );
        std::queue<int> q;
        q.push( src );
        seen[src] = 1;
        pred.assign( vert_cnt, -1 );

        while (!q.empty()) {
            int cur = q.front(); q.pop();
            for (int nxt = 0; nxt < vert_cnt; ++nxt) {
                if (!seen[nxt] && residual[cur][nxt] > 0) {
                    seen[nxt] = 1;
                    pred[nxt] = cur;
                    if (nxt == dst) return true;
                    q.push( nxt );
                }
            }
        }
        return false;
    };

    // Edmonds‑Karp max‑flow using the bfs_path lambda
    auto max_flow = [&](int src, int dst,
                        const std::vector<std::vector<int>>& cap_matrix) -> int {
        std::vector<std::vector<int>> residual = cap_matrix;
        std::vector<int> parent( vert_cnt );
        int total = 0;

        while ( bfs_path( src, dst, residual, parent ) ) {
            int bottleneck = 1e9;
            for (int v = dst; v != src; v = parent[v]) {
                int u = parent[v];
                if (residual[u][v] < bottleneck) bottleneck = residual[u][v];
            }
            for (int v = dst; v != src; v = parent[v]) {
                int u = parent[v];
                residual[u][v] -= bottleneck;
                residual[v][u] += bottleneck;
            }
            total += bottleneck;
        }
        return total;
    };

    // ---------- Gomory‑Hu tree construction ----------
    std::vector<int> ancestor( vert_cnt, 0 );
    std::vector<int> edge_w( vert_cnt, 0 );
    for (int v = 1; v < vert_cnt; ++v) {
        int s = v;
        int t = ancestor[v];
        int flow_val = max_flow( s, t, base_cap );
        edge_w[v] = flow_val;

        // residual after last max‑flow run
        std::vector<std::vector<int>> residual = base_cap;
        // recompute residual to obtain reachable set
        // (reuse max_flow's internal residual logic)
        {
            std::vector<std::vector<int>> tmp = base_cap;
            std::vector<int> dummy;
            while ( bfs_path( s, t, tmp, dummy ) ) {
                int bottleneck = 1e9;
                for (int x = t; x != s; x = dummy[x]) {
                    int y = dummy[x];
                    if (tmp[y][x] < bottleneck) bottleneck = tmp[y][x];
                }
                for (int x = t; x != s; x = dummy[x]) {
                    int y = dummy[x];
                    tmp[y][x] -= bottleneck;
                    tmp[x][y] += bottleneck;
                }
            }
            residual.swap( tmp );
        }

        std::vector<int> reachable( vert_cnt, 0 );
        std::queue<int> q;
        q.push( s );
        reachable[s] = 1;
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            for (int nxt = 0; nxt < vert_cnt; ++nxt) {
                if (!reachable[nxt] && residual[cur][nxt] > 0) {
                    reachable[nxt] = 1;
                    q.push( nxt );
                }
            }
        }

        for (int w = v + 1; w < vert_cnt; ++w) {
            if (ancestor[w] == t && reachable[w]) ancestor[w] = v;
        }
        if (reachable[ ancestor[t] ]) {
            int old = ancestor[t];
            ancestor[t] = v;
            ancestor[v] = old;
            edge_w[t] = edge_w[v];
            edge_w[v] = flow_val;
        }
    }

    // ---------- build adjacency list of the tree ----------
    std::vector<std::vector<std::pair<int,int>>> tree_adj( vert_cnt );
    for (int i = 1; i < vert_cnt; ++i) {
        tree_adj[i].push_back( { ancestor[i], edge_w[i] } );
        tree_adj[ ancestor[i] ].push_back( { i, edge_w[i] } );
    }

    // ---------- compute all‑pairs min‑cut via tree ----------
    std::vector<std::vector<int>> result( vert_cnt, std::vector<int>( vert_cnt, 0 ) );

    // helper lambda: min edge on path between a and b in the tree
    auto min_on_path = [&](int a, int b) -> int {
        std::vector<int> seen( vert_cnt, 0 );
        std::vector<int> best( vert_cnt, 0 );
        std::queue<int> qq;
        qq.push( a );
        seen[a] = 1;
        best[a] = 1e9;

        while (!qq.empty()) {
            int cur = qq.front(); qq.pop();
            if (cur == b) break;
            for (auto &pr : tree_adj[cur]) {
                int nxt = pr.first;
                int wgt = pr.second;
                if (!seen[nxt]) {
                    seen[nxt] = 1;
                    best[nxt] = (best[cur] < wgt) ? best[cur] : wgt;
                    qq.push( nxt );
                }
            }
        }
        return best[b];
    };

    for (int i = 0; i < vert_cnt; ++i) {
        for (int j = i + 1; j < vert_cnt; ++j) {
            int val = min_on_path( i, j );
            result[i][j] = result[j][i] = val;
        }
    }

    // ---------- output ----------
    std::cout << "All‑Pairs Edge Connectivity (matrix):\n";
    for (int i = 0; i < vert_cnt; ++i) {
        for (int j = 0; j < vert_cnt; ++j) {
            std::cout << result[i][j] << (j + 1 == vert_cnt ? '\n' : ' ');
        }
    }
    return 0;
}
