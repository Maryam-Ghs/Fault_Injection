/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cstdlib>
#include <ctime>

class GraphReverser {
private:
    // Maximum numbers – keep them modest for stack allocation
    int maxVertices;
    int maxEdges;

    // Original adjacency list structures
    int originHead[100];      // head of list for each vertex
    int originTo[200];        // destination vertex of each edge
    int originNext[200];      // next edge index in the list
    int originEdgeCnt;        // total edges added

    // Reversed adjacency list structures
    int revHead[100];
    int revTo[200];
    int revNext[200];
    int revEdgeCnt;

public:
    // Constructor – sets sizes and clears all heads
    GraphReverser(int v, int e) {
        maxVertices = v;
        maxEdges    = e;

        // Initialise heads to -1 (empty list)
        int idx = 0;
        while (idx < maxVertices) {
            originHead[idx] = -1;
            revHead[idx]    = -1;
            idx = idx + 1;
        }

        // No edges yet
        originEdgeCnt = 0;
        revEdgeCnt    = 0;
    }

    // Add a directed edge u -> w to the original list
    void addEdge(int u, int w) {
        // Store edge information at the current position
        originTo[originEdgeCnt]   = w;
        originNext[originEdgeCnt] = originHead[u];
        originHead[u]             = originEdgeCnt;

        // Move to next slot
        originEdgeCnt = originEdgeCnt + 1;
    }

    // Build the reversed adjacency list from the original
    void buildReversed() {
        int eIdx = 0;
        // Walk through every edge in the original list
        while (eIdx < originEdgeCnt) {
            int src = findSource(eIdx);   // original source vertex
            int dst = originTo[eIdx];      // original destination vertex

            // Insert reversed edge dst -> src into rev list
            revTo[revEdgeCnt]   = src;
            revNext[revEdgeCnt] = revHead[dst];
            revHead[dst]        = revEdgeCnt;

            revEdgeCnt = revEdgeCnt + 1;
            eIdx = eIdx + 1;
        }
    }

    // Helper: locate the source vertex of a given edge index
    int findSource(int edgeIdx) {
        int v = 0;
        // Scan vertex heads to see which list contains edgeIdx
        while (v < maxVertices) {
            int cur = originHead[v];
            while (cur != -1) {
                if (cur == edgeIdx) {
                    return v;
                }
                cur = originNext[cur];
            }
            v = v + 1;
        }
        // Should never reach here for valid indices
        return -1;
    }

    // Print adjacency list for a given set of arrays
    void printList(const char* title,
                   int headArr[100],
                   int toArr[200],
                   int nextArr[200]) {
        std::cout << title << std::endl;
        int v = 0;
        while (v < maxVertices) {
            std::cout << "  Vertex " << v << ":";
            int cur = headArr[v];
            while (cur != -1) {
                std::cout << " " << toArr[cur];
                cur = nextArr[cur];
            }
            std::cout << std::endl;
            v = v + 1;
        }
        std::cout << std::endl;
    }

    // Public interface to display both graphs
    void display() {
        printList("Original adjacency list:", originHead, originTo, originNext);
        printList("Reversed adjacency list:", revHead, revTo, revNext);
    }
};

int main() {
    // --------------------------------------------------------------
    // 1. Define graph size (sparse, skewed input)
    // --------------------------------------------------------------
    int vertexCount = 20;   // many vertices
    int edgeCount   = 8;    // few edges, clustered on early vertices

    // --------------------------------------------------------------
    // 2. Create GraphReverser instance
    // --------------------------------------------------------------
    GraphReverser g(vertexCount, edgeCount);

    // --------------------------------------------------------------
    // 3. Add deterministic skewed edges (clustered on vertices 0,1,2)
    // --------------------------------------------------------------
    int edgeList[8][2] = {
        {0, 3},
        {0, 5},
        {0, 9},
        {1, 4},
        {1, 7},
        {1, 12},
        {2, 15},
        {2, 18}
    };
    for (int i = 0; i < edgeCount; ++i) {
        g.addEdge(edgeList[i][0], edgeList[i][1]);
    }

    // --------------------------------------------------------------
    // 4. Build the reversed adjacency list
    // --------------------------------------------------------------
    g.buildReversed();

    // --------------------------------------------------------------
    // 5. Print both adjacency lists
    // --------------------------------------------------------------
    g.display();

    return 0;
}
