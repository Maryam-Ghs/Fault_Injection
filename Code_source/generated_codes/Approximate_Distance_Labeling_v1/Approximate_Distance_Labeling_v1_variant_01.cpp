#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

// version #1

class ApproxDistLabeler {
public:
    // dimensions
    int nodeCnt;
    int edgeCnt;
    int dimLbl;

    // graph representation
    std::vector< std::vector<int> > adjList;

    // label vectors
    std::vector< std::vector<float> > nodeLbl;

    // constructor
    ApproxDistLabeler(int n, int e, int d) {
        nodeCnt = n;
        edgeCnt = e;
        dimLbl  = d;
        adjList.assign(nodeCnt, std::vector<int>());
        nodeLbl.assign(nodeCnt, std::vector<float>(dimLbl));
    }

    // -----------------------------------------------------------------
    // step 1 : create a random undirected graph
    void buildRandomGraph() {
        int created = 0;
        while (created < edgeCnt) {
            int a = std::rand() % nodeCnt;
            int b = std::rand() % nodeCnt;
            if (a == b) continue;                     // avoid loops
            if (hasEdge(a, b)) continue;              // avoid multi‑edges
            adjList[a].push_back(b);
            adjList[b].push_back(a);
            ++created;
        }
    }

    // helper to check existence of an edge
    bool hasEdge(int u, int v) {
        for (size_t i = 0; i < adjList[u].size(); ++i)
            if (adjList[u][i] == v) return true;
        return false;
    }

    // -----------------------------------------------------------------
    // step 2 : assign random labels (multi‑step computation)
    void assignRandomLabels() {
        int i = 0;
        while (i < nodeCnt) {
            int j = 0;
            while (j < dimLbl) {
                // multi‑step: generate int, cast to float, scale
                int raw = std::rand() % 1000;
                float fval = static_cast<float>(raw);
                nodeLbl[i][j] = fval / 1000.0f;   // now in [0,1)
                ++j;
            }
            ++i;
        }
    }

    // -----------------------------------------------------------------
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

    // -----------------------------------------------------------------
    // step 4 : approximate distance from label vectors
    float approxDist(int u, int v) {
        float sum = 0.0f;
        int idx = 0;
        while (idx < dimLbl) {
            float diff = nodeLbl[u][idx] - nodeLbl[v][idx];
            sum += diff * diff;
            ++idx;
        }
        // expanded sqrt step
        float res = sum;
        // Newton‑Raphson iteration (one step) for sqrt
        if (res > 0.0f) {
            float guess = res / 2.0f;
            guess = 0.5f * (guess + res / guess);
            res = guess;
        }
        return res;
    }

    // -----------------------------------------------------------------
    // step 5 : test a set of random node pairs
    void evaluate() {
        const int testPairs = 1;
        int trials = 0;
        while (trials < testPairs) {
            int s = std::rand() % nodeCnt;
            int t = std::rand() % nodeCnt;
            if (s == t) continue;

            // exact distance
            std::vector<int> dvec = bfsDist(s);
            int exact = dvec[t];

            // approximate distance
            float approx = approxDist(s, t);

            // output
            std::cout << "pair (" << s << "," << t << "): "
                      << "exact=" << exact << " "
                      << "approx=" << approx << "\n";

            ++trials;
        }
    }
};

// ---------------------------------------------------------------------
// driver
int main() {
    std::srand(0); // deterministic seed for minimal‑boundary test

    // parameters (minimal valid sizes)
    int nodes   = 2;   // number of vertices
    int edges   = 1;   // number of edges
    int dimLab  = 1;   // label dimension

    ApproxDistLabeler labeler(nodes, edges, dimLab);

    // generate graph
    labeler.buildRandomGraph();

    // generate labels
    labeler.assignRandomLabels();

    // evaluate approximation on random pairs
    labeler.evaluate();

    return 0;
}
