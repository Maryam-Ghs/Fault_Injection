/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <vector>

class Graph {
public:
    int vertexTotal;
    int edgeTotal;
    std::vector< std::vector<int> > links;
    std::vector<int> visitedFlag;
    std::vector<int> discoveryTime;
    std::vector<int> lowLink;
    std::vector<int> parentNode;
    std::vector<int> articulationFlag;
    int timer;

    Graph(int v, int e) {
        vertexTotal = v;
        edgeTotal   = e;
        links.resize(vertexTotal);
        visitedFlag.resize(vertexTotal);
        discoveryTime.resize(vertexTotal);
        lowLink.resize(vertexTotal);
        parentNode.resize(vertexTotal);
        articulationFlag.resize(vertexTotal);
        timer = 0;
    }

    void addEdge(int a, int b) {
        links[a].push_back(b);
        links[b].push_back(a);
    }

    void dfsArticulation(int current) {
        visitedFlag[current] = 1;
        timer = timer + 1;
        discoveryTime[current] = timer;
        lowLink[current] = timer;
        int childCount = 0;
        int neighbourIdx = 0;
        while (neighbourIdx < (int)links[current].size()) {
            int neighbour = links[neighbourIdx];
            if (visitedFlag[neighbour] == 0) {
                parentNode[neighbour] = current;
                childCount = childCount + 1;
                dfsArticulation(neighbour);
                int lowFromChild = lowLink[neighbour];
                int lowCurrent   = lowLink[current];
                if (lowFromChild < lowCurrent) lowLink[current] = lowFromChild;
                if (parentNode[current] != -1) {
                    int lowChild = lowLink[neighbour];
                    int discCurr = discoveryTime[current];
                    if (lowChild >= discCurr) articulationFlag[current] = 1;
                }
            } else if (neighbour != parentNode[current]) {
                int discNeighbour = discoveryTime[neighbour];
                int lowCurrent    = lowLink[current];
                if (discNeighbour < lowCurrent) lowLink[current] = discNeighbour;
            }
            neighbourIdx = neighbourIdx + 1;
        }
        if (parentNode[current] == -1 && childCount > 1) {
            articulationFlag[current] = 1;
        }
    }

    void findArticulations() {
        int i = 0;
        while (i < vertexTotal) {
            visitedFlag[i] = 0;
            parentNode[i]   = -1;
            articulationFlag[i] = 0;
            i = i + 1;
        }
        timer = 0;
        int start = 0;
        while (start < vertexTotal) {
            if (visitedFlag[start] == 0) {
                dfsArticulation(start);
            }
            start = start + 1;
        }
    }

    void printGraph() {
        std::cout << "Adjacency list of the generated graph:\n";
        int v = 0;
        while (v < vertexTotal) {
            std::cout << "  Vertex " << v << ": ";
            int j = 0;
            while (j < (int)links[v].size()) {
                std::cout << links[v][j] << " ";
                j = j + 1;
            }
            std::cout << "\n";
            v = v + 1;
        }
    }

    void outputArticulations() {
        std::cout << "Articulation points (by vertex index):\n";
        int v = 0;
        bool any = false;
        while (v < vertexTotal) {
            if (articulationFlag[v] == 1) {
                std::cout << "  " << v;
                any = true;
            }
            v = v + 1;
        }
        if (!any) std::cout << "  None";
        std::cout << "\n";
    }
};

int main() {
    int vertices = 15;
    int edges    = 20; // deterministic pseudo‑random pattern
    Graph myGraph(vertices, edges);
    // deterministic pseudo‑random edge list
    myGraph.addEdge(0, 1);
    myGraph.addEdge(0, 2);
    myGraph.addEdge(0, 5);
    myGraph.addEdge(1, 3);
    myGraph.addEdge(1, 4);
    myGraph.addEdge(2, 3);
    myGraph.addEdge(2, 6);
    myGraph.addEdge(3, 7);
    myGraph.addEdge(4, 5);
    myGraph.addEdge(4, 8);
    myGraph.addEdge(5, 9);
    myGraph.addEdge(6, 7);
    myGraph.addEdge(6,10);
    myGraph.addEdge(7,11);
    myGraph.addEdge(8, 9);
    myGraph.addEdge(8,12);
    myGraph.addEdge(9,13);
    myGraph.addEdge(10,11);
    myGraph.addEdge(10,14);
    myGraph.addEdge(11,12);
    // total 20 edges
    myGraph.printGraph();
    myGraph.findArticulations();
    myGraph.outputArticulations();
    return 0;
}
