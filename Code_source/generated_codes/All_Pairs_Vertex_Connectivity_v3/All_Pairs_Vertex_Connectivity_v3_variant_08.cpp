/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <queue>
#include <algorithm>

class VertexConnector {
public:
    // ----- configuration -----
    int V;                    // original vertex count
    int N;                    // split vertex count (2*V)
    int INF;                  // large enough value for "infinite" capacity

    // ----- graph storage (stack arrays) -----
    int cap[16][16];          // capacity matrix (max 2*8 vertices)
    int res[16][16];          // residual matrix
    int level[16];            // BFS level / visited marker

    // ----- constructor -----
    VertexConnector(int vcnt) {
        V = vcnt;
        N = V * 2;
        INF = 1000000;        // no const, just a mutable int
        // clear matrices
        for (int i = 0; i < 16; ++i)
            for (int j = 0; j < 16; ++j)
                cap[i][j] = 0;
    }

    // ----- add original undirected edge (u,v) -----
    void addEdgeOrig(int u, int v) {
        // split vertices: u_in = 2*u, u_out = 2*u+1
        int u_in  = u * 2;
        int u_out = u * 2 + 1;
        int v_in  = v * 2;
        int v_out = v * 2 + 1;

        // capacity of vertex itself (except later for source/sink)
        cap[u_in][u_out] = 1;
        cap[v_in][v_out] = 1;

        // undirected edge becomes two directed infinite edges
        cap[u_out][v_in] = INF;
        cap[v_out][u_in] = INF;
    }

    // ----- prepare residual matrix for a new (s,t) pair -----
    void resetResidual(int src, int dst) {
        // copy capacity to residual, but adjust source/sink vertex capacities
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                res[i][j] = cap[i][j];

        // source and sink vertices get infinite internal capacity
        int s_in = src * 2;
        int s_out = src * 2 + 1;
        int t_in = dst * 2;
        int t_out = dst * 2 + 1;

        res[s_in][s_out] = INF;
        res[t_in][t_out] = INF;
    }

    // ----- Edmonds‑Karp max‑flow (manual loop unrolling) -----
    int maxFlow(int s, int t) {
        int flow = 0;
        int parent[16];

        while (true) {
            // ----- BFS to find augmenting path -----
            for (int i = 0; i < N; ++i) level[i] = 0;
            std::queue<int> q;
            q.push(s);
            level[s] = 1;
            for (int i = 0; i < N; ++i) parent[i] = -1;

            while (!q.empty()) {
                int cur = q.front(); q.pop();

                // manual unroll of neighbor scan (N ≤ 8)
                if (N > 0 && level[0] == 0 && res[cur][0] > 0) { level[0] = 1; parent[0] = cur; q.push(0); }
                if (N > 1 && level[1] == 0 && res[cur][1] > 0) { level[1] = 1; parent[1] = cur; q.push(1); }
                if (N > 2 && level[2] == 0 && res[cur][2] > 0) { level[2] = 1; parent[2] = cur; q.push(2); }
                if (N > 3 && level[3] == 0 && res[cur][3] > 0) { level[3] = 1; parent[3] = cur; q.push(3); }
                if (N > 4 && level[4] == 0 && res[cur][4] > 0) { level[4] = 1; parent[4] = cur; q.push(4); }
                if (N > 5 && level[5] == 0 && res[cur][5] > 0) { level[5] = 1; parent[5] = cur; q.push(5); }
                if (N > 6 && level[6] == 0 && res[cur][6] > 0) { level[6] = 1; parent[6] = cur; q.push(6); }
                if (N > 7 && level[7] == 0 && res[cur][7] > 0) { level[7] = 1; parent[7] = cur; q.push(7); }
                if (N > 8 && level[8] == 0 && res[cur][8] > 0) { level[8] = 1; parent[8] = cur; q.push(8); }
                if (N > 9 && level[9] == 0 && res[cur][9] > 0) { level[9] = 1; parent[9] = cur; q.push(9); }
                if (N >10 && level[10]== 0 && res[cur][10]> 0) { level[10]= 1; parent[10]= cur; q.push(10);}
                if (N >11 && level[11]== 0 && res[cur][11]> 0) { level[11]= 1; parent[11]= cur; q.push(11);}
                if (N >12 && level[12]== 0 && res[cur][12]> 0) { level[12]= 1; parent[12]= cur; q.push(12);}
                if (N >13 && level[13]== 0 && res[cur][13]> 0) { level[13]= 1; parent[13]= cur; q.push(13);}
                if (N >14 && level[14]== 0 && res[cur][14]> 0) { level[14]= 1; parent[14]= cur; q.push(14);}
                if (N >15 && level[15]== 0 && res[cur][15]> 0) { level[15]= 1; parent[15]= cur; q.push(15);}
            }

            if (parent[t] == -1) break;          // no augmenting path

            // ----- find bottleneck -----
            int inc = INF;
            int v = t;
            while (v != s) {
                int u = parent[v];
                // reorder: subtract before compare (still correct)
                int cand = res[u][v];
                inc = (cand < inc) ? cand : inc;
                v = u;
            }

            // ----- augment flow -----
            v = t;
            while (v != s) {
                int u = parent[v];
                res[u][v] = res[u][v] - inc;
                res[v][u] = res[v][u] + inc;
                v = u;
            }
            flow = flow + inc;                  // reorder: addition after increment
        }
        return flow;
    }

    // ----- compute all‑pairs vertex connectivity -----
    void computeAll() {
        // result matrix on stack
        int conn[8][8];   // max original vertices = 8 (split → 16)
        // manual unroll for each source
        for (int src = 0; src < V; ++src) {
            for (int dst = 0; dst < V; ++dst) {
                if (src == dst) {
                    conn[src][dst] = 0;
                    continue;
                }
                // source is src_out, sink is dst_in (any consistent choice works)
                int s = src * 2 + 1;   // src_out
                int t = dst * 2;       // dst_in
                resetResidual(src, dst);
                conn[src][dst] = maxFlow(s, t);
            }
        }

        // ----- print matrix -----
        std::cout << "All‑Pairs Vertex Connectivity (V=" << V << ")\n";
        for (int i = 0; i < V; ++i) {
            for (int j = 0; j < V; ++j) {
                std::cout << conn[i][j];
                if (j < V - 1) std::cout << ' ';
            }
            std::cout << '\n';
        }
    }
};

int main() {
    // ----- deterministic sparse‑skewed test graph (V = 8) -----
    // vertices: 0..7
    // edges: a central hub 0 connected to 1,2,3; a small component 4‑5; another isolated pair 6‑7
    VertexConnector solver(8);
    solver.addEdgeOrig(0, 1);
    solver.addEdgeOrig(0, 2);
    solver.addEdgeOrig(0, 3);
    solver.addEdgeOrig(4, 5);
    solver.addEdgeOrig(6, 7);

    solver.computeAll();

    return 0;
}
