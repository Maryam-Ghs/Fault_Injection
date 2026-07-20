#include <iostream>
#include <vector>
#include <queue>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

// deterministic pseudo‑random generator (linear congruential)
static unsigned int deterministic_rand_state = 123456789;
inline int deterministic_rand() {
    deterministic_rand_state = deterministic_rand_state * 1103515245 + 12345;
    return (deterministic_rand_state & 0x7fffffff);
}

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
    // step 1 : create a deterministic undirected graph
    void buildRandomGraph() {
        int created = 0;
        while (created < edgeCnt) {
            int a = deterministic_rand() % nodeCnt;
            int b = deterministic_rand() % nodeCnt;
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
    // step 2 : assign deterministic labels (multi‑step computation)
    void assignRandomLabels() {
        int i = 0;
        while (i < nodeCnt) {
            int j = 0;
            while (j < dimLbl) {
                // multi‑step: generate int, cast to float, scale
                int raw = deterministic_rand() % 1000;
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
    // step 5 : test a set of deterministic node pairs
    void evaluate() {
        const int testPairs = 10;
        int trials = 0;
        while (trials < testPairs) {
            int s = deterministic_rand() % nodeCnt;
            int t = deterministic_rand() % nodeCnt;
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
    // deterministic seed already set in generator

    // parameters (medium‑size deterministic arrays)
    int nodes   = 467;          // number of vertices
    int edges   = 3500;         // number of edges
    int dimLab  = 9;            // label dimension

    ApproxDistLabeler labeler(nodes, edges, dimLab);

    // generate graph
    labeler.buildRandomGraph();

    // generate labels
    labeler.assignRandomLabels();

    // evaluate approximation on deterministic pairs
    labeler.evaluate();

    return 0;
}
