/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>

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
    // step 1 : create a deterministic path graph (worst‑case depth)
    void buildRandomGraph() {
        for (int i = 0; i < nodeCnt - 1; ++i) {
            adjList[i].push_back(i + 1);
            adjList[i + 1].push_back(i);
        }
        // edgeCnt may be larger than needed; extra edges are ignored in this variant
    }

    // helper to check existence of an edge
    bool hasEdge(int u, int v) {
        for (size_t i = 0; i < adjList[u].size(); ++i)
            if (adjList[u][i] == v) return true;
        return false;
    }

    // -----------------------------------------------------------------
    // step 2 : assign deterministic decreasing labels (reverse‑ordered)
    void assignRandomLabels() {
        for (int i = 0; i < nodeCnt; ++i) {
            for (int j = 0; j < dimLbl; ++j) {
                // label decreases with node index, slight variation per dimension
                float base = static_cast<float>(nodeCnt - i) / static_cast<float>(nodeCnt);
                float dimFactor = 1.0f - static_cast<float>(j) * 0.01f; // small tweak per dimension
                nodeLbl[i][j] = base * dimFactor;
            }
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
    // step 5 : test a set of deterministic reverse‑ordered node pairs
    void evaluate() {
        const int testPairs = 10;
        int trials = 0;
        while (trials < testPairs) {
            int s = nodeCnt - 1 - trials; // high index
            int t = trials;               // low index
            if (s == t) {
                t = (t + 1) % nodeCnt;
            }

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
    // parameters (deterministic worst‑case)
    int nodes   = 500;          // number of vertices
    int edges   = nodes - 1;    // path graph edge count
    int dimLab  = 8;            // label dimension

    ApproxDistLabeler labeler(nodes, edges, dimLab);

    // generate deterministic path graph
    labeler.buildRandomGraph();

    // generate deterministic decreasing labels
    labeler.assignRandomLabels();

    // evaluate approximation on reverse‑ordered pairs
    labeler.evaluate();

    return 0;
}
