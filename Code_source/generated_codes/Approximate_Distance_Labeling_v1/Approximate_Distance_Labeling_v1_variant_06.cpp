#include <iostream>
#include <vector>
#include <queue>
#include <cmath>

/* LLM input variant 6: ordered-structured */

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
    // step 1 : create a deterministic structured graph (simple path)
    void buildRandomGraph() {
        // create a path graph: 0-1-2-...-(nodeCnt-1)
        for (int i = 0; i < nodeCnt - 1 && edgeCnt > 0; ++i) {
            adjList[i].push_back(i + 1);
            adjList[i + 1].push_back(i);
            --edgeCnt;
        }
        // if more edges are requested, add symmetric next‑nearest connections
        int offset = 2;
        while (edgeCnt > 0) {
            for (int i = 0; i + offset < nodeCnt && edgeCnt > 0; ++i) {
                adjList[i].push_back(i + offset);
                adjList[i + offset].push_back(i);
                --edgeCnt;
            }
            ++offset;
        }
    }

    // helper to check existence of an edge
    bool hasEdge(int u, int v) {
        for (size_t i = 0; i < adjList[u].size(); ++i)
            if (adjList[u][i] == v) return true;
        return false;
    }

    // -----------------------------------------------------------------
    // step 2 : assign deterministic, ordered labels
    void assignRandomLabels() {
        for (int i = 0; i < nodeCnt; ++i) {
            for (int j = 0; j < dimLbl; ++j) {
                // monotonic increasing values across nodes and dimensions
                nodeLbl[i][j] = static_cast<float>(i * dimLbl + j) / static_cast<float>(nodeCnt * dimLbl);
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
    // step 5 : test a set of deterministic node pairs
    void evaluate() {
        const int testPairs = 10;
        for (int i = 0; i < testPairs; ++i) {
            int s = i % nodeCnt;
            int t = (i + 1) % nodeCnt;
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
        }
    }
};

// ---------------------------------------------------------------------
// driver
int main() {
    // parameters (ordered, structured data)
    int nodes   = 100;   // number of vertices
    int edges   = 150;   // number of edges (will be filled deterministically)
    int dimLab  = 8;     // label dimension

    ApproxDistLabeler labeler(nodes, edges, dimLab);

    // generate deterministic structured graph
    labeler.buildRandomGraph();

    // generate deterministic labels
    labeler.assignRandomLabels();

    // evaluate approximation on deterministic pairs
    labeler.evaluate();

    return 0;
}
