#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

int main()
{
    // ------------------------------------------------------------
    //  Version #4 – All‑Pairs Vertex Connectivity (loop‑heavy, heap‑based)
    // ------------------------------------------------------------

    // ---- generate a random undirected graph --------------------
    int verticesCount = 10;                    // size of the graph (larger to show sparsity)
    int maxEdgesProb = 5;                      // very low probability (0‑100) of an edge

    // adjacency matrix of the original graph (0‑1 values)
    int *edgeMatrix = new int[verticesCount * verticesCount];
    std::srand((unsigned)std::time(0));

    for (int a = 0; a < verticesCount; ++a)
    {
        for (int b = 0; b < verticesCount; ++b)
        {
            if (a == b)
                edgeMatrix[a * verticesCount + b] = 0;
            else
            {
                int rnd = std::rand() % 100;
                edgeMatrix[a * verticesCount + b] = (rnd < maxEdgesProb) ? 1 : 0;
            }
        }
    }

    // ---- create a small dense cluster to introduce skewness ----
    // vertices 0,1,2 will form a fully connected subgraph
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (i == j) continue;
            edgeMatrix[i * verticesCount + j] = 1;
            edgeMatrix[j * verticesCount + i] = 1; // ensure undirected symmetry
        }
    }

    // ---- helper: build a flow network for a given (src,dst) ----
    // each vertex v (except src,dst) is split into v_in and v_out.
    // capacity of the split edge is 1 (vertex removal cost).
    // edges of the original graph become infinite‑capacity arcs
    // between the appropriate split parts.

    int totalNodes = 2 * verticesCount;               // v_in = v, v_out = v+verticesCount
    int INF = verticesCount + 5;                       // “infinite” capacity

    // allocate flow capacity matrix (will be rebuilt for every pair)
    int *flowCap = new int[totalNodes * totalNodes];

    // result matrix (vertex connectivity for every ordered pair)
    int *connectivity = new int[verticesCount * verticesCount];

    // ---- Edmonds‑Karp (BFS augmenting path) --------------------
    auto bfs = [&](int src, int sink, int *parent) -> bool
    {
        // initialise visited array
        int *visited = new int[totalNodes];
        for (int i = 0; i < totalNodes; ++i) visited[i] = 0;

        // queue implementation with static array
        int *q = new int[totalNodes];
        int head = 0, tail = 0;

        q[tail++] = src;
        visited[src] = 1;
        parent[src] = -1;

        while (head < tail)
        {
            int cur = q[head++];
            for (int nxt = 0; nxt < totalNodes; ++nxt)
            {
                int residual = flowCap[cur * totalNodes + nxt];
                if (visited[nxt] == 0 && residual > 0)
                {
                    q[tail++] = nxt;
                    parent[nxt] = cur;
                    visited[nxt] = 1;
                    if (nxt == sink)
                    {
                        delete[] visited;
                        delete[] q;
                        return true;
                    }
                }
            }
        }
        delete[] visited;
        delete[] q;
        return false;
    };

    // ---- main double loop over all ordered pairs ---------------
    for (int src = 0; src < verticesCount; ++src)
    {
        for (int dst = 0; dst < verticesCount; ++dst)
        {
            if (src == dst)
            {
                connectivity[src * verticesCount + dst] = 0;
                continue;
            }

            // ---- rebuild flow network for this (src,dst) ----------
            // reset capacities
            for (int i = 0; i < totalNodes * totalNodes; ++i)
                flowCap[i] = 0;

            // split vertices: capacity 1 from v_in to v_out (except src/dst)
            for (int v = 0; v < verticesCount; ++v)
            {
                if (v == src || v == dst) continue;
                int in  = v;
                int out = v + verticesCount;
                flowCap[in * totalNodes + out] = 1;          // vertex capacity
            }

            // translate original edges into infinite‑capacity arcs
            for (int u = 0; u < verticesCount; ++u)
            {
                for (int v = 0; v < verticesCount; ++v)
                {
                    if (edgeMatrix[u * verticesCount + v] == 0) continue;

                    // from u_out to v_in
                    int from = (u == src || u == dst) ? u : u + verticesCount;
                    int   to = (v == src || v == dst) ? v : v;
                    flowCap[from * totalNodes + to] = INF;

                    // because the graph is undirected, also the opposite direction
                    int revFrom = (v == src || v == dst) ? v : v + verticesCount;
                    int   revTo = (u == src || u == dst) ? u : u;
                    flowCap[revFrom * totalNodes + revTo] = INF;
                }
            }

            // source and sink are the original vertices (no split)
            int sourceNode = src;
            int sinkNode   = dst;

            // ---- run Edmonds‑Karp -------------------------------
            int maxFlow = 0;
            int *parent = new int[totalNodes];

            while (bfs(sourceNode, sinkNode, parent))
            {
                // find bottleneck (will always be 1 because vertex caps are 1)
                int pathCap = INF;
                int v = sinkNode;
                while (v != sourceNode)
                {
                    int u = parent[v];
                    int residual = flowCap[u * totalNodes + v];
                    if (residual < pathCap) pathCap = residual;
                    v = u;
                }

                // augment the path
                v = sinkNode;
                while (v != sourceNode)
                {
                    int u = parent[v];
                    flowCap[u * totalNodes + v] -= pathCap;
                    flowCap[v * totalNodes + u] += pathCap;
                    v = u;
                }
                maxFlow += pathCap;
            }

            delete[] parent;

            connectivity[src * verticesCount + dst] = maxFlow;
        }
    }

    // ---- print the vertex‑connectivity matrix -----------------
    std::cout << "All‑Pairs Vertex Connectivity (size " << verticesCount << ")\n";
    for (int i = 0; i < verticesCount; ++i)
    {
        for (int j = 0; j < verticesCount; ++j)
        {
            std::cout << connectivity[i * verticesCount + j] << ' ';
        }
        std::cout << '\n';
    }

    // ---- clean up ------------------------------------------------
    delete[] edgeMatrix;
    delete[] flowCap;
    delete[] connectivity;

    return 0;
}
