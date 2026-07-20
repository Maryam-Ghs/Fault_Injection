/* LLM input variant 5: duplicate-heavy */

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
    // ---- generate deterministic duplicate‑heavy directed graph -----------------
    const int V = 1000;          // vertices
    const int E = 5000;          // edges (capacity, multiple of 4)
    Graph* g = new Graph(V, E);

    // Fill with repeated edge patterns using manual unrolling (4 edges per loop)
    int generated = 0;
    while (generated < E) {
        int a0 = 0, b0 = 1;   // edge 0->1
        int a1 = 1, b1 = 2;   // edge 1->2
        int a2 = 0, b2 = 1;   // edge 0->1 (duplicate)
        int a3 = 1, b3 = 2;   // edge 1->2 (duplicate)

        // Avoid self‑loops (none in this pattern)
        g->addEdge(a0, b0);
        g->addEdge(a1, b1);
        g->addEdge(a2, b2);
        g->addEdge(a3, b3);
        generated += 4;
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
