// Articulation Component Graph – version #7
// ------------------------------------------------------------
// This program generates a random undirected graph,
// finds all articulation points using a DFS based algorithm,
// and prints the result. All numeric types are int or float,
// no const, double, long, unsigned or cin are used.
// ------------------------------------------------------------

/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class Graph {
public:
    // ----- data members ------------------------------------------------
    int vertexTotal;                     // number of vertices
    int edgeTotal;                       // number of edges
    std::vector< std::vector<int> > links; // adjacency list

    // ----- algorithmic helpers -----------------------------------------
    std::vector<int> visitedFlag;       // 0 = not visited, 1 = visited
    std::vector<int> discoveryTime;     // discovery time of each vertex
    std::vector<int> lowLink;           // low-link value of each vertex
    std::vector<int> parentNode;        // parent in DFS tree
    std::vector<int> articulationFlag;  // 1 = articulation point
    int timer;                           // global discovery timer

    // ----- constructor --------------------------------------------------
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

    // ----- add a single undirected edge --------------------------------
    void addEdge(int a, int b) {
        links[a].push_back(b);
        links[b].push_back(a);
    }

    // ----- generate a random simple graph --------------------------------
    void generateRandom() {
        int created = 0;
        while (created < edgeTotal) {
            int src = std::rand() % vertexTotal;
            int dst = std::rand() % vertexTotal;
            if (src == dst) continue;                     // no self‑loop
            // avoid parallel edges
            bool already = false;
            int idx = 0;
            while (idx < (int)links[src].size()) {
                if (links[src][idx] == dst) { already = true; break; }
                idx = idx + 1;
            }
            if (already) continue;
            addEdge(src, dst);
            created = created + 1;
        }
    }

    // ----- DFS utility for articulation points ---------------------------
    void dfsArticulation(int current) {
        // mark visited
        visitedFlag[current] = 1;

        // set discovery and low values
        timer = timer + 1;
        discoveryTime[current] = timer;
        lowLink[current] = timer;

        // count of children in DFS tree
        int childCount = 0;

        // iterate over neighbours using while loop
        int neighbourIdx = 0;
        while (neighbourIdx < (int)links[current].size()) {
            int neighbour = links[current][neighbourIdx];

            if (visitedFlag[neighbour] == 0) {
                // tree edge
                parentNode[neighbour] = current;
                childCount = childCount + 1;

                dfsArticulation(neighbour);

                // after recursion: update low-link
                int lowFromChild = lowLink[neighbour];
                int lowCurrent   = lowLink[current];
                if (lowFromChild < lowCurrent) lowLink[current] = lowFromChild;

                // articulation condition for non‑root
                if (parentNode[current] != -1) {
                    int lowChild = lowLink[neighbour];
                    int discCurr = discoveryTime[current];
                    if (lowChild >= discCurr) articulationFlag[current] = 1;
                }
            }
            else if (neighbour != parentNode[current]) {
                // back edge
                int discNeighbour = discoveryTime[neighbour];
                int lowCurrent    = lowLink[current];
                if (discNeighbour < lowCurrent) lowLink[current] = discNeighbour;
            }

            neighbourIdx = neighbourIdx + 1;
        }

        // articulation condition for root
        if (parentNode[current] == -1 && childCount > 1) {
            articulationFlag[current] = 1;
        }
    }

    // ----- driver to find all articulation points -----------------------
    void findArticulations() {
        // initialise helpers
        int i = 0;
        while (i < vertexTotal) {
            visitedFlag[i] = 0;
            parentNode[i]   = -1;
            articulationFlag[i] = 0;
            i = i + 1;
        }
        timer = 0;

        // start DFS from every unvisited vertex (handles disconnected graphs)
        int start = 0;
        while (start < vertexTotal) {
            if (visitedFlag[start] == 0) {
                dfsArticulation(start);
            }
            start = start + 1;
        }
    }

    // ----- print the graph (optional) -----------------------------------
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

    // ----- output the articulation points --------------------------------
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

// ---------------------------------------------------------------------
// main – generate a large‑sized random graph and find its articulations
// ---------------------------------------------------------------------
int main() {
    std::srand((unsigned)std::time(0));

    // large‑safe‑stress input: 2000 vertices, 5000 edges
    int vertices = 2000;
    int edges    = 5000;

    Graph myGraph(vertices, edges);
    myGraph.generateRandom();

    // optional: show the graph structure (may be lengthy)
    myGraph.printGraph();

    // compute articulation points
    myGraph.findArticulations();

    // display results
    myGraph.outputArticulations();

    return 0;
}
