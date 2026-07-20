/* LLM input variant 9: medium-deterministic-random */
// Version 7 – Adjacency List Reversal (heap‑based, manual unrolling, fused ops)

#include <iostream>
#include <cstdlib>

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

// Simple deterministic pseudo‑random generator (LCG) with fixed seed
static unsigned int lcg_state = 123456789u;
inline unsigned int lcg_next() {
    lcg_state = lcg_state * 1103515245u + 12345u;
    return (lcg_state >> 16) & 0x7FFFu;   // 15‑bit result
}

int main() {
    // ---- generate deterministic medium‑sized directed graph ------------------
    const int V = 823;          // vertices (prime for diversity)
    const int E = 4096;         // edges (capacity)
    Graph* g = new Graph(V, E);

    // Fill with deterministic pseudo‑random edges using manual unrolling (4 edges per loop)
    int generated = 0;
    while (generated < E) {
        int a0 = lcg_next() % V;
        int b0 = lcg_next() % V;
        int a1 = lcg_next() % V;
        int b1 = lcg_next() % V;
        int a2 = lcg_next() % V;
        int b2 = lcg_next() % V;
        int a3 = lcg_next() % V;
        int b3 = lcg_next() % V;

        // Avoid self‑loops for clarity
        if (a0 != b0) g->addEdge(a0, b0);
        if (a1 != b1) g->addEdge(a1, b1);
        if (a2 != b2) g->addEdge(a2, b2);
        if (a3 != b3) g->addEdge(a3, b3);
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
