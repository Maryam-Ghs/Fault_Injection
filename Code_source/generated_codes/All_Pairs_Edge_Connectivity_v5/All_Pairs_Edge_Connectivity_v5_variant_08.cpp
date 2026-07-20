/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cstdlib>
#include <ctime>

class EdgeConnectAll {
public:
    // maximum vertices we handle (stack allocation)
    int maxV;
    // adjacency matrix for capacities
    int cap[30][30];

    // constructor creates a deterministic sparse undirected graph
    EdgeConnectAll(int v) {
        maxV = v;
        // initialise matrix with zeros
        int i = 0;
        while (i < maxV) {
            int j = 0;
            while (j < maxV) {
                cap[i][j] = 0;
                ++j;
            }
            ++i;
        }

        // manually add a few sparse edges with varied capacities
        // cluster 1: vertices 0‑3 densely connected
        cap[0][1] = 5; cap[1][0] = 5;
        cap[0][2] = 3; cap[2][0] = 3;
        cap[1][3] = 4; cap[3][1] = 4;
        cap[2][3] = 2; cap[3][2] = 2;
        // isolated small component: vertices 4‑5
        cap[4][5] = 7; cap[5][4] = 7;
        // far‑apart edges creating skew
        cap[8][9] = 1; cap[9][8] = 1;
        cap[10][11] = 6; cap[11][10] = 6;
        // long‑range low‑capacity edge
        cap[0][11] = 2; cap[11][0] = 2;
    }

    // Breadth‑first search for augmenting path, stores parent in par[]
    bool bfs(int src, int dst, int par[], int residual[][30]) {
        bool seen[30];
        int q[30];
        int head = 0, tail = 0;

        // initialise visited
        int i = 0;
        while (i < maxV) {
            seen[i] = false;
            ++i;
        }

        q[tail++] = src;
        seen[src] = true;
        par[src] = -1;

        while (head < tail) {
            int cur = q[head++];
            int nxt = 0;
            while (nxt < maxV) {
                if (!seen[nxt] && residual[cur][nxt] > 0) {
                    q[tail++] = nxt;
                    par[nxt] = cur;
                    seen[nxt] = true;
                }
                ++nxt;
            }
            if (seen[dst]) break;
        }
        return seen[dst];
    }

    // Edmonds‑Karp computes max flow (hence min cut) between s and t
    int maxFlow(int s, int t) {
        // build residual graph copy
        int residual[30][30];
        int i = 0;
        while (i < maxV) {
            int j = 0;
            while (j < maxV) {
                residual[i][j] = cap[i][j];
                ++j;
            }
            ++i;
        }

        int parent[30];
        int flow = 0;

        // repeat while there is an augmenting path
        while (bfs(s, t, parent, residual)) {
            // find bottleneck capacity on the path
            int bottleneck = 1e9;
            int v = t;
            while (v != s) {
                int u = parent[v];
                int curCap = residual[u][v];
                if (curCap < bottleneck) bottleneck = curCap;
                v = u;
            }

            // augment flow along the path
            v = t;
            while (v != s) {
                int u = parent[v];
                int before = residual[u][v];
                residual[u][v] = before - bottleneck;
                int revBefore = residual[v][u];
                residual[v][u] = revBefore + bottleneck;
                v = u;
            }

            // add bottleneck to total flow
            flow = flow + bottleneck;
        }
        return flow;
    }

    // compute all‑pairs edge connectivity matrix
    void computeAll() {
        int result[30][30];
        int i = 0;
        while (i < maxV) {
            int j = 0;
            while (j < maxV) {
                result[i][j] = 0;
                ++j;
            }
            ++i;
        }

        i = 0;
        while (i < maxV) {
            int j = i + 1;
            while (j < maxV) {
                int mf = maxFlow(i, j);
                result[i][j] = mf;
                result[j][i] = mf;
                ++j;
            }
            ++i;
        }

        // print matrix
        std::cout << "All‑Pairs Edge Connectivity (min‑cut values):\n";
        i = 0;
        while (i < maxV) {
            int j = 0;
            while (j < maxV) {
                std::cout << result[i][j] << (j + 1 == maxV ? '\n' : ' ');
                ++j;
            }
            ++i;
        }
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    // deterministic sparse graph with 12 vertices
    int vertices = 12;
    EdgeConnectAll solver(vertices);
    solver.computeAll();

    return 0;
}
