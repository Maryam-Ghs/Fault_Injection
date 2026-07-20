/* LLM input variant 3: zeros-and-ones */
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
    int vertices = 3;
    int edges    = 2;
    Graph myGraph(vertices, edges);
    // edges emphasizing zeros and ones
    myGraph.addEdge(0, 1);
    myGraph.addEdge(1, 2);
    myGraph.printGraph();
    myGraph.findArticulations();
    myGraph.outputArticulations();
    return 0;
}
