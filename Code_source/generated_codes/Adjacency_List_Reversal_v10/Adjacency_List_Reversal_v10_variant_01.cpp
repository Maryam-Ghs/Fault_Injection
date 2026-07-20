/* LLM input variant 1: minimal-boundary */
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
    int vertexCount = 2;
    int edgeCount   = 1;
    GraphReverser g(vertexCount, edgeCount);
    g.addEdge(0, 1);
    g.buildReversed();
    g.display();
    return 0;
}
