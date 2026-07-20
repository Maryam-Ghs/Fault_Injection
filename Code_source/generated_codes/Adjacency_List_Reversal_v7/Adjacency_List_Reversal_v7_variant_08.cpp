/* LLM input variant 8: sparse-skewed */
// Version 7 – Adjacency List Reversal (heap‑based, manual unrolling, fused ops)

#include <iostream>
#include <cstdlib>
#include <ctime>

class Graph {
    int  vertexCnt;      // number of vertices
    int  edgeCap;        // capacity for edges
    int* head;           // start index of adjacency list for each vertex
    int* nxt;            // linked‑list next pointer for edges
    int* to;             // destination vertex of each edge
    int  edgePos;        // next free position in edge arrays

public:
    // Constructor: allocate all structures on the heap
    Graph(int v, int e) : vertexCnt(v), edgeCap(e), edgePos(0) {
        head = new int[vertexCnt];
        nxt  = new int[edgeCap];
        to   = new int[edgeCap];
        for (int i = 0; i < vertexCnt; ++i) head[i] = -1;
    }

    // Destructor: free heap memory
    ~Graph() {
        delete[] head;
        delete[] nxt;
        delete[] to;
    }

    // Insert edge u -> w (fused assignment, manual unrolling not needed here)
    inline void addEdge(int u, int w) {
        int idx = edgePos++;
        to[idx]   = w;
        nxt[idx]  = head[u];
        head[u]   = idx;
    }

    // Build reversed graph (manual loop unrolling, fused ops)
    Graph* reversed() const {
        Graph* rev = new Graph(vertexCnt, edgePos);
        // Iterate over all vertices
        for (int src = 0; src < vertexCnt; ++src) {
            int cur = head[src];
            // Process two edges per iteration when possible
            while (cur != -1) {
                int dst   = to[cur];
                rev->addEdge(dst, src);          // reversed edge

                // Advance to next edge (first)
                cur = nxt[cur];
                if (cur == -1) break;            // no second edge

                // Process second edge in the same loop body
                dst   = to[cur];
                rev->addEdge(dst, src);
                cur = nxt[cur];
            }
        }
        return rev;
    }

    // Print adjacency list (simple format)
    void dump(const char* title) const {
        std::cout << title << "\n";
        for (int v = 0; v < vertexCnt; ++v) {
            std::cout << v << ":";
            for (int e = head[v]; e != -1; e = nxt[e])
                std::cout << ' ' << to[e];
            std::cout << '\n';
        }
    }
};

int main() {
    // ---- generate sparse, skewed directed graph ---------------------------------
    const int V = 1000;          // vertices
    const int E = 200;           // edges (capacity and actual count)
    Graph* g = new Graph(V, E);

    // Concentrate edges on the first 10 vertices, each connecting to 20 distinct targets
    int edgeCount = 0;
    for (int src = 0; src < 10 && edgeCount < E; ++src) {
        for (int i = 0; i < 20 && edgeCount < E; ++i) {
            int dst = (src * 20 + i + 1) % V;   // deterministic target, ensures spread
            if (src != dst) {
                g->addEdge(src, dst);
                ++edgeCount;
            }
        }
    }

    // ---- display original graph (first 10 vertices only for brevity) ----------
    g->dump("Original adjacency lists (first 10 vertices):");

    // ---- reverse the graph ----------------------------------------------------
    Graph* rev = g->reversed();

    // ---- display reversed graph (first 10 vertices only) ----------------------
    rev->dump("\nReversed adjacency lists (first 10 vertices):");

    // ---- clean up -------------------------------------------------------------
    delete g;
    delete rev;
    return 0;
}
