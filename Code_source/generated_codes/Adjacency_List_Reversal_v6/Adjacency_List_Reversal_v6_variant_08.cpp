#include <iostream>

/* LLM input variant 8: sparse-skewed */

////////////////////////////////////////////////////////////
// Version #6 - Adjacency List Reversal (stack‑based)
// Simple, readable, class‑based implementation
////////////////////////////////////////////////////////////

class Graph {
public:
    // maximum limits (stack allocation)
    int vertexCount;          // number of vertices
    int edgeLimit;            // maximum edges that can be stored
    int headIdx[30];          // head of adjacency list for each vertex
    int toNode[60];           // destination vertex of each edge
    int nextEdge[60];         // next edge index in adjacency list
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
    // ---- create a sparse, heavily skewed directed graph ----
    int nodes = 12;                 // number of vertices (larger set)
    int maxEdges = 20;             // capacity for edges (mostly unused)
    Graph original(nodes, maxEdges);

    // Skewed edges: many outgoing from vertex 0, few or none from others
    original.addEdge(0, 1);
    original.addEdge(0, 2);
    original.addEdge(0, 3);
    original.addEdge(0, 4);
    original.addEdge(0, 5);
    original.addEdge(0, 6);
    original.addEdge(0, 7);
    original.addEdge(0, 8);
    original.addEdge(0, 9);
    original.addEdge(0, 10);
    // Add a couple of edges elsewhere to keep it minimally connected
    original.addEdge(2, 11);
    original.addEdge(5, 11);

    // ---- display original adjacency list ----
    original.display("Original adjacency list:");

    // ---- compute and display reversed adjacency list ----
    Graph reversed = original.reverse();
    reversed.display("Reversed adjacency list:");

    return 0;
}
