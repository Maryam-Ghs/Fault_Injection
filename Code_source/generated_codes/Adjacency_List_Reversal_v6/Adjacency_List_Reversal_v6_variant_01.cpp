#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

////////////////////////////////////////////////////////////
// Version #6 - Adjacency List Reversal (stack‑based)
// Simple, readable, class‑based implementation
////////////////////////////////////////////////////////////

class Graph {
public:
    // maximum limits (stack allocation)
    int vertexCount;          // number of vertices
    int edgeLimit;            // maximum edges that can be stored
    int headIdx[20];          // head of adjacency list for each vertex
    int toNode[40];           // destination vertex of each edge
    int nextEdge[40];         // next edge index in adjacency list
    int edgePtr;              // next free position in edge arrays

    // Constructor: initialise structures
    Graph(int v, int e) {
        vertexCount = v;
        edgeLimit   = e;
        edgePtr = 0;
        int i = 0;
        while (i < vertexCount) {
            headIdx[i] = -1;   // -1 denotes end of list
            i = i + 1;
        }
    }

    // Add a directed edge u -> v
    void addEdge(int u, int v) {
        if (edgePtr >= edgeLimit) return;          // safety guard
        int cur = edgePtr;                         // store current position
        edgePtr = edgePtr + 1;                     // advance pointer

        toNode[cur]   = v;                         // destination
        nextEdge[cur] = headIdx[u];                // link previous head
        headIdx[u]    = cur;                       // new head is this edge
    }

    // Produce a new graph that is the reversal of this one
    Graph reverse() {
        Graph rev(vertexCount, edgeLimit);         // new empty graph
        int v = 0;
        while (v < vertexCount) {
            int eIdx = headIdx[v];
            while (eIdx != -1) {
                int dest = toNode[eIdx];
                rev.addEdge(dest, v);              // reverse direction
                eIdx = nextEdge[eIdx];
            }
            v = v + 1;
        }
        return rev;
    }

    // Print adjacency list
    void display(const char* title) {
        std::cout << title << "\n";
        int v = 0;
        while (v < vertexCount) {
            std::cout << "Vertex " << v << ": ";
            int eIdx = headIdx[v];
            while (eIdx != -1) {
                std::cout << toNode[eIdx] << " ";
                eIdx = nextEdge[eIdx];
            }
            std::cout << "\n";
            v = v + 1;
        }
        std::cout << std::endl;
    }
};

int main() {
    // Seed random generator (kept for compatibility)
    std::srand(static_cast<unsigned>(std::time(0)));

    // ---- generate a minimal non‑trivial directed graph ----
    int nodes = 2;                 // number of vertices
    int maxEdges = 1;              // capacity for edges
    Graph original(nodes, maxEdges);

    // deterministic single edge 0 -> 1
    original.addEdge(0, 1);

    // ---- display original adjacency list ----
    original.display("Original adjacency list:");

    // ---- compute and display reversed adjacency list ----
    Graph reversed = original.reverse();
    reversed.display("Reversed adjacency list:");

    return 0;
}
