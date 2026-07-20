#include <iostream>
#include <vector>
#include <queue>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

class ApproxDistLabeler {
public:
    int nodeCnt;
    int edgeCnt;
    int dimLbl;

    std::vector< std::vector<int> > adjList;
    std::vector< std::vector<float> > nodeLbl;

    ApproxDistLabeler(int n, int e, int d) {
        nodeCnt = n;
        edgeCnt = e;
        dimLbl  = d;
        adjList.assign(nodeCnt, std::vector<int>());
        nodeLbl.assign(nodeCnt, std::vector<float>(dimLbl));
    }

    // step 1 : create a deterministic undirected graph with many duplicate patterns
    void buildRandomGraph() {
        int created = 0;
        while (created < edgeCnt) {
            int a = created % nodeCnt;
            int b = (a + 1) % nodeCnt; // connect each node to its next neighbor, forming a cycle
            if (a == b) { ++created; continue; }
            if (hasEdge(a, b)) { ++created; continue; }
            adjList[a].push_back(b);
            adjList[b].push_back(a);
            ++created;
        }
    }

    bool hasEdge(int u, int v) {
        for (size_t i = 0; i < adjList[u].size(); ++i)
            if (adjList[u][i] == v) return true;
        return false;
    }

    // step 2 : assign identical labels to all nodes (duplicate-heavy)
    void assignRandomLabels() {
        for (int i = 0; i < nodeCnt; ++i) {
            for (int j = 0; j < dimLbl; ++j) {
                nodeLbl[i][j] = 0.333f; // same value for every dimension of every node
            }
        }
    }

    // step 3 : exact shortest‑path distances (BFS for unweighted graph)
    std::vector<int> bfsDist(int src) {
        std::vector<int> dist(nodeCnt, -1);
        std::queue<int> q;
        dist[src] = 0;
        q.push(src);
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            for (size_t k = 0; k < adjList[cur].size(); ++k) {
                int nxt = adjList[cur][k];
                if (dist[nxt] == -1) {
                    dist[nxt] = dist[cur] + 1;
                    q.push(nxt);
                }
            }
        }
        return dist;
    }

    // step 4 : approximate distance from label vectors
    float approxDist(int u, int v) {
        float sum = 0.0f;
        for (int idx = 0; idx < dimLbl; ++idx) {
            float diff = nodeLbl[u][idx] - nodeLbl[v][idx];
            sum += diff * diff;
        }
        float res = sum;
        if (res > 0.0f) {
            float guess = res / 2.0f;
            guess = 0.5f * (guess + res / guess);
            res = guess;
        }
        return res;
    }

    // step 5 : test a set of deterministic node pairs with duplicates
    void evaluate() {
        const int testPairs = 5;
        int pairList[testPairs][2] = {
            {0, 1},
            {0, 1}, // duplicate pair
            {2, 3},
            {2, 3}, // duplicate pair
            {4, 5}
        };
        for (int i = 0; i < testPairs; ++i) {
            int s = pairList[i][0];
            int t = pairList[i][1];
            if (s == t) continue;

            std::vector<int> dvec = bfsDist(s);
            int exact = dvec[t];
            float approx = approxDist(s, t);

            std::cout << "pair (" << s << "," << t << "): "
                      << "exact=" << exact << " "
                      << "approx=" << approx << "\n";
        }
    }
};

int main() {
    // deterministic small graph with duplicate-heavy characteristics
    int nodes  = 6;   // number of vertices
    int edges  = 6;   // number of edges (forming a cycle)
    int dimLab = 3;   // label dimension

    ApproxDistLabeler labeler(nodes, edges, dimLab);

    labeler.buildRandomGraph();
    labeler.assignRandomLabels();
    labeler.evaluate();

    return 0;
}
