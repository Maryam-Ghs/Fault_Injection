#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

class EdgeConnectAll {
public:
    // maximum vertices we handle (stack allocation)
    int maxV;
    // adjacency matrix for capacities
    int cap[30][30];

    // constructor creates a deterministic undirected graph with mixed signed capacities
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

        // fill with deterministic capacities ranging from -3 to 3 for i < j, make symmetric
        i = 0;
        while (i < maxV) {
            int j = i + 1;
            while (j < maxV) {
                int val = ((i + j) % 7) - 3; // yields -3 .. 3
                cap[i][j] = val;
                cap[j][i] = val;
                ++j;
            }
            ++i;
        }
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

    // deterministic graph size (≤ 20 for stack safety)
    int vertices = 9;
    EdgeConnectAll solver(vertices);
    solver.computeAll();

    return 0;
}
