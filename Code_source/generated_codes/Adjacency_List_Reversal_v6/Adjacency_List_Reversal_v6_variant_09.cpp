#include <iostream>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

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
    // ---- generate a medium‑sized deterministic directed graph ----
    const int nodes = 10;                // number of vertices
    const int maxEdges = 30;             // capacity for edges
    Graph original(nodes, maxEdges);

    // deterministic edge set (no self‑loops)
    const int edgeList[][2] = {
        {0,1}, {0,2}, {1,3}, {2,3}, {3,4},
        {4,5}, {5,6}, {6,7}, {7,8}, {8,9},
        {9,0}, {2,5}, {3,7}, {1,8}, {6,2},
        {5,0}, {4,2}, {7,1}, {8,3}, {9,5}
    };
    const int edgeCount = sizeof(edgeList) / sizeof(edgeList[0]);

    for (int i = 0; i < edgeCount && i < maxEdges; ++i) {
        int from = edgeList[i][0];
        int to   = edgeList[i][1];
        if (from != to) {
            original.addEdge(from, to);
        }
    }

    // ---- display original adjacency list ----
    original.display("Original adjacency list:");

    // ---- compute and display reversed adjacency list ----
    Graph reversed = original.reverse();
    reversed.display("Reversed adjacency list:");

    return 0;
}
