/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cstdlib>
#include <ctime>

class GraphReverser {
private:
    int maxVertices;
    int maxEdges;
    int originHead[100];
    int originTo[200];
    int originNext[200];
    int originEdgeCnt;
    int revHead[100];
    int revTo[200];
    int revNext[200];
    int revEdgeCnt;

public:
    GraphReverser(int v, int e) {
        maxVertices = v;
        maxEdges    = e;
        int idx = 0;
        while (idx < maxVertices) {
            originHead[idx] = -1;
            revHead[idx]    = -1;
            idx = idx + 1;
        }
        originEdgeCnt = 0;
        revEdgeCnt    = 0;
    }

    void addEdge(int u, int w) {
        originTo[originEdgeCnt]   = w;
        originNext[originEdgeCnt] = originHead[u];
        originHead[u]             = originEdgeCnt;
        originEdgeCnt = originEdgeCnt + 1;
    }

    void buildReversed() {
        int eIdx = 0;
        while (eIdx < originEdgeCnt) {
            int src = findSource(eIdx);
            int dst = originTo[eIdx];
            revTo[revEdgeCnt]   = src;
            revNext[revEdgeCnt] = revHead[dst];
            revHead[dst]        = revEdgeCnt;
            revEdgeCnt = revEdgeCnt + 1;
            eIdx = eIdx + 1;
        }
    }

    int findSource(int edgeIdx) {
        int v = 0;
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
        return -1;
    }

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

    void display() {
        printList("Original adjacency list:", originHead, originTo, originNext);
        printList("Reversed adjacency list:", revHead, revTo, revNext);
    }
};

int main() {
    int vertexCount = 6;
    int edgeCount   = 12;
    GraphReverser g(vertexCount, edgeCount);

    // Deterministic, reverse‑ordered edge set designed to stress findSource
    // Sources are listed from highest to lowest vertex, avoiding self‑loops.
    const int edges[12][2] = {
        {5, 0}, {5, 1}, {5, 2},
        {4, 0}, {4, 1}, {4, 2},
        {3, 0}, {3, 1}, {3, 2},
        {2, 0}, {2, 1},
        {1, 0}
    };

    for (int i = 0; i < edgeCount; ++i) {
        int u = edges[i][0];
        int w = edges[i][1];
        g.addEdge(u, w);
    }

    g.buildReversed();
    g.display();
    return 0;
}
