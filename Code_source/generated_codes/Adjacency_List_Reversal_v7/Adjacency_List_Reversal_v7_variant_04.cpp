// LLM input variant 4: signed-extremes
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
    // ---- deterministic edge list with mixed signed values --------------------
    const int V = 6;               // vertices (positive)
    const int E = 12;              // number of edge entries (capacity)
    Graph* g = new Graph(V, E);

    // Edge definitions: include negative, zero, and positive values
    const int edgeList[E][2] = {
        {0, 1},    // positive
        {-1, 2},   // negative source (ignored)
        {3, 0},    // positive
        {4, -2},   // negative destination (ignored)
        {2, 2},    // self‑loop (ignored)
        {5, 0},    // positive source out of range (ignored)
        {0, 0},    // self‑loop (ignored)
        {-3, -3},  // both negative (ignored)
        {1, 4},    // positive
        {2, 5},    // destination out of range (ignored)
        {0, 3},    // positive
        {3, 3}     // self‑loop (ignored)
    };

    int added = 0;
    for (int i = 0; i < E; ++i) {
        int src = edgeList[i][0];
        int dst = edgeList[i][1];
        // Accept only valid, non‑self‑loop edges within vertex range
        if (src >= 0 && dst >= 0 && src < V && dst < V && src != dst) {
            g->addEdge(src, dst);
            ++added;
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
