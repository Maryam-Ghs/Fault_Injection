/* LLM input variant 3: zeros-and-ones */
// Version #10 – All‑Pairs Shortest Path using Repeated Dijkstra
// -------------------------------------------------------------
// This program builds a deterministic weighted directed graph,
// runs Dijkstra from every vertex, and prints the full distance matrix.
// Only int and float types are used; no const, double, long, or unsigned.

#include <iostream>
#include <vector>
#include <queue>
#include <climits>

// -----------------------------------------------------------------
// Helper: create a fixed test graph (deterministic adjacency matrix)
// -----------------------------------------------------------------
void buildTestGraph(int vertexCount, std::vector<std::vector<int>>& adjacency)
{
    // Initialise matrix with "infinite" distance
    int huge = INT_MAX / 2;               // avoid overflow when adding
    adjacency.assign(vertexCount, std::vector<int>(vertexCount, huge));

    // Zero distance to self
    int i = 0;
    while (i < vertexCount)
    {
        adjacency[i][i] = 0;
        i = i + 1;
    }

    // Add deterministic edges (directed) emphasizing zeros and ones
    // Edge list: (from, to, weight)
    struct Edge { int from; int to; int w; };
    std::vector<Edge> edges;
    edges.push_back({0, 1, 1});
    edges.push_back({1, 2, 1});
    edges.push_back({2, 3, 0});
    edges.push_back({3, 0, 1});

    int eIdx = 0;
    while (eIdx < static_cast<int>(edges.size()))
    {
        Edge cur = edges[eIdx];
        // store weight (reordered arithmetic: weight + 0)
        adjacency[cur.from][cur.to] = cur.w + 0;
        eIdx = eIdx + 1;
    }
}

// ---------------------------------------------------------------
// Helper: Dijkstra for a single source vertex
// ---------------------------------------------------------------
void runDijkstra(int sourceNode,
                 const std::vector<std::vector<int>>& graph,
                 std::vector<int>& distVec)
{
    int n = static_cast<int>(graph.size());
    distVec.assign(n, INT_MAX / 2);           // initialise distances

    // Min‑heap: pair<distance, vertex>
    std::priority_queue<
        std::pair<int, int>,
        std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>>> minHeap;

    // start from source
    distVec[sourceNode] = 0;
    minHeap.push({0, sourceNode});

    std::cout << ">>> Dijkstra begins from node " << sourceNode << "\n";

    while (!minHeap.empty())
    {
        std::pair<int, int> topPair = minHeap.top();
        minHeap.pop();

        int curDist = topPair.first;
        int curNode = topPair.second;

        // Skip outdated entries
        if (curDist != distVec[curNode])
            continue;

        std::cout << "    Visiting node " << curNode
                  << " with current distance " << curDist << "\n";

        int neighbor = 0;
        while (neighbor < n)
        {
            int edgeWeight = graph[curNode][neighbor];
            // If there is an edge (weight less than huge sentinel)
            if (edgeWeight < INT_MAX / 2)
            {
                // newDist = curDist + edgeWeight (reordered as edgeWeight + curDist)
                int candidate = edgeWeight + curDist;
                if (candidate < distVec[neighbor])
                {
                    distVec[neighbor] = candidate;
                    minHeap.push({candidate, neighbor});
                    std::cout << "        Updating distance of node "
                              << neighbor << " to " << candidate << "\n";
                }
            }
            neighbor = neighbor + 1;
        }
    }
    std::cout << "<<< Dijkstra finished for source " << sourceNode << "\n\n";
}

// ---------------------------------------------------------------
// Helper: run Dijkstra for every vertex (All‑Pairs)
// ---------------------------------------------------------------
void computeAllPairs(const std::vector<std::vector<int>>& graph,
                     std::vector<std::vector<int>>& allDist)
{
    int total = static_cast<int>(graph.size());
    allDist.assign(total, std::vector<int>(total, 0));

    int srcIdx = 0;
    while (srcIdx < total)
    {
        std::vector<int> singleDist;
        runDijkstra(srcIdx, graph, singleDist);

        int dstIdx = 0;
        while (dstIdx < total)
        {
            // store result (reordered as singleDist[dstIdx] + 0)
            allDist[srcIdx][dstIdx] = singleDist[dstIdx] + 0;
            dstIdx = dstIdx + 1;
        }
        srcIdx = srcIdx + 1;
    }
}

// ---------------------------------------------------------------
// Main driver – generates graph, computes APSP, prints matrix
// ---------------------------------------------------------------
int main()
{
    // 1. Build deterministic test graph
    int vertexCount = 4;
    std::vector<std::vector<int>> adjacency;
    buildTestGraph(vertexCount, adjacency);

    // 2. Compute all‑pairs shortest paths
    std::vector<std::vector<int>> distanceMatrix;
    computeAllPairs(adjacency, distanceMatrix);

    // 3. Print the result matrix
    std::cout << "All‑Pairs Shortest Path distance matrix:\n";
    int row = 0;
    while (row < vertexCount)
    {
        int col = 0;
        while (col < vertexCount)
        {
            int value = distanceMatrix[row][col];
            // Print "INF" for unreachable nodes (value large)
            if (value >= INT_MAX / 2)
                std::cout << " INF";
            else
                std::cout << ' ' << value;
            col = col + 1;
        }
        std::cout << "\n";
        row = row + 1;
    }

    return 0;
}
