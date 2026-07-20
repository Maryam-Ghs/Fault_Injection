// LLM input variant 9: medium-deterministic-random
// Approximate Distance via Landmark Sampling – version #6
// ------------------------------------------------------
// Implements a tiny graph, selects landmarks, runs Dijkstra from each landmark,
// and estimates distances between all node pairs using the landmark sums.
//
// Restrictions:
//   * only int and float types
//   * no const, double, long, unsigned
//   * heap allocation with new / delete
//   * no cin, input generated inside the program
//   * simple, readable style, reordered arithmetic where possible
// ------------------------------------------------------

#include <iostream>
#include <cstdio>
#include <cstdlib>

// ---------- Graph definition ----------
struct Edge {
    int to;        // target vertex
    float w;       // edge weight
    int nxt;       // next edge index in adjacency list
};

class Graph {
public:
    int V;                 // number of vertices
    int Emax;              // maximum number of directed edges
    int *head;             // adjacency list heads
    Edge *edges;           // edge storage
    int ecnt;              // current edge count

    Graph(int verts, int maxEdges) {
        V = verts;
        Emax = maxEdges;
        head = new int[V];
        edges = new Edge[Emax];
        ecnt = 0;
        int i = 0;
        while (i < V) {
            head[i] = -1;
            i = i + 1;
        }
    }

    ~Graph() {
        delete[] head;
        delete[] edges;
    }

    void link(int a, int b, float w) {
        // forward edge
        edges[ecnt].to = b;
        edges[ecnt].w = w;
        edges[ecnt].nxt = head[a];
        head[a] = ecnt;
        ecnt = ecnt + 1;
        // backward edge (undirected graph)
        edges[ecnt].to = a;
        edges[ecnt].w = w;
        edges[ecnt].nxt = head[b];
        head[b] = ecnt;
        ecnt = ecnt + 1;
    }
};

// ---------- Dijkstra (simple O(V^2)) ----------
void dijkstra(const Graph &g, int src, float *dist) {
    int V = g.V;
    int *used = new int[V];
    int i = 0;
    while (i < V) {
        used[i] = 0;
        dist[i] = 1e30f;          // effectively infinity
        i = i + 1;
    }
    dist[src] = 0.0f;

    int processed = 0;
    while (processed < V) {
        // find unvisited vertex with minimal distance
        int v = -1;
        float best = 1e30f;
        i = 0;
        while (i < V) {
            if (!used[i] && dist[i] < best) {
                best = dist[i];
                v = i;
            }
            i = i + 1;
        }
        if (v == -1) break;               // remaining vertices unreachable
        used[v] = 1;
        processed = processed + 1;

        // relax edges from v
        int e = g.head[v];
        while (e != -1) {
            int to = g.edges[e].to;
            float w = g.edges[e].w;
            // reordered addition: (dist[v] + w) instead of (w + dist[v])
            float nd = dist[v] + w;
            if (nd < dist[to]) {
                dist[to] = nd;
            }
            e = g.edges[e].nxt;
        }
    }
    delete[] used;
}

// ---------- Approximate distance computation ----------
class ApproxDist {
public:
    Graph *g;                     // pointer to the graph
    int Lcnt;                     // number of landmarks
    int *land;                    // landmark vertex ids
    float **labDist;              // distances from each landmark

    ApproxDist(Graph *graph, int lmCount) {
        g = graph;
        Lcnt = lmCount;
        land = new int[Lcnt];
        labDist = new float*[Lcnt];
        int i = 0;
        while (i < Lcnt) {
            // pick first Lcnt vertices as landmarks (edge‑case: could be isolated)
            land[i] = i;
            labDist[i] = new float[g->V];
            i = i + 1;
        }
        computeLandmarks();
    }

    ~ApproxDist() {
        int i = 0;
        while (i < Lcnt) {
            delete[] labDist[i];
            i = i + 1;
        }
        delete[] labDist;
        delete[] land;
    }

    void computeLandmarks() {
        int i = 0;
        while (i < Lcnt) {
            dijkstra(*g, land[i], labDist[i]);
            i = i + 1;
        }
    }

    // estimate distance between a and b
    float query(int a, int b) {
        float best = 1e30f;
        int i = 0;
        while (i < Lcnt) {
            // reordered sum: labDist[i][a] + labDist[i][b]
            float cand = labDist[i][a] + labDist[i][b];
            if (cand < best) best = cand;
            i = i + 1;
        }
        return best;
    }
};

// ---------- Main driver ----------
int main() {
    // Deterministic medium‑sized graph:
    //   - 10 vertices (0..9)
    //   - pseudo‑random edge weights, plus zero‑weight, huge‑weight, and self‑loops
    int V = 10;
    int maxE = V * V + 10;               // generous capacity for extra edges
    Graph *g = new Graph(V, maxE);

    // Generate a dense base graph with deterministic pseudo‑random weights
    int i = 0;
    while (i < V) {
        int j = i + 1;
        while (j < V) {
            // weight cycles through 1.0 .. 23.0 in a deterministic pattern
            float w = ((i * 7 + j * 3) % 23) + 1.0f;
            g->link(i, j, w);
            j = j + 1;
        }
        i = i + 1;
    }

    // Add zero‑weight edges
    g->link(0, 1, 0.0f);
    g->link(2, 3, 0.0f);

    // Add huge‑weight edges
    g->link(0, V - 1, 1000000.0f);
    g->link(1, V - 2, 999999.0f);

    // Add self‑loops (will be ignored by Dijkstra)
    g->link(4, 4, 5.0f);
    g->link(9, 9, 10.0f);

    // landmark count (choose 3 for a richer approximation)
    int L = 3;
    ApproxDist *approx = new ApproxDist(g, L);

    // Print approximate distance matrix
    std::printf("Approximate distance matrix (landmarks = %d):\n", L);
    i = 0;
    while (i < V) {
        int j = 0;
        while (j < V) {
            float d = approx->query(i, j);
            // reorder printing: cast to int for readability (floats may be large)
            std::printf("%8.1f", d);
            j = j + 1;
        }
        std::printf("\n");
        i = i + 1;
    }

    delete approx;
    delete g;
    return 0;
}
