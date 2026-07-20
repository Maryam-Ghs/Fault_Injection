#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */
int main() {
    // Version #8 - adjacency list reversal with manual loop unrolling
    // --------------------------------------------------------------
    // Generate a deterministic directed graph with adversarial structure
    int vertices = 1000;                 // number of vertices
    int edges    = 5000;                 // number of edges

    std::srand(12345);                  // fixed seed for reproducibility (kept for consistency)

    // Edge lists (source -> destination)
    int* src = new int[edges];
    int* dst = new int[edges];

    // Fill edge lists with deterministic adversarial data:
    // Every edge points to vertex 0, source vertices cycle through 0..vertices-1.
    int i = 0;
    while (i + 3 < edges) {
        src[i]     = i % vertices;
        dst[i]     = 0;
        src[i + 1] = (i + 1) % vertices;
        dst[i + 1] = 0;
        src[i + 2] = (i + 2) % vertices;
        dst[i + 2] = 0;
        src[i + 3] = (i + 3) % vertices;
        dst[i + 3] = 0;
        i += 4;
    }
    while (i < edges) {
        src[i] = i % vertices;
        dst[i] = 0;
        ++i;
    }

    // ---------- Build original adjacency list (flattened) ----------
    int* outCnt = new int[vertices];
    for (int v = 0; v < vertices; ++v) outCnt[v] = 0;

    // Count outgoing edges (loop unrolled)
    i = 0;
    while (i + 3 < edges) {
        ++outCnt[src[i]];
        ++outCnt[src[i + 1]];
        ++outCnt[src[i + 2]];
        ++outCnt[src[i + 3]];
        i += 4;
    }
    while (i < edges) {
        ++outCnt[src[i]];
        ++i;
    }

    // Prefix sums -> start positions
    int* outPos = new int[vertices + 1];
    outPos[0] = 0;
    for (int v = 0; v < vertices; ++v) {
        outPos[v + 1] = outPos[v] + outCnt[v];
    }

    // Allocate flattened adjacency array
    int* outAdj = new int[edges];
    // Temporary cursor array
    int* curPos = new int[vertices];
    for (int v = 0; v < vertices; ++v) curPos[v] = outPos[v];

    // Fill adjacency list (unrolled)
    i = 0;
    while (i + 3 < edges) {
        outAdj[curPos[src[i]]++] = dst[i];
        outAdj[curPos[src[i + 1]]++] = dst[i + 1];
        outAdj[curPos[src[i + 2]]++] = dst[i + 2];
        outAdj[curPos[src[i + 3]]++] = dst[i + 3];
        i += 4;
    }
    while (i < edges) {
        outAdj[curPos[src[i]]++] = dst[i];
        ++i;
    }

    // ---------- Build reversed adjacency list ----------
    int* revCnt = new int[vertices];
    for (int v = 0; v < vertices; ++v) revCnt[v] = 0;

    // Count incoming edges (which become outgoing in the reversed graph)
    i = 0;
    while (i + 3 < edges) {
        ++revCnt[dst[i]];
        ++revCnt[dst[i + 1]];
        ++revCnt[dst[i + 2]];
        ++revCnt[dst[i + 3]];
        i += 4;
    }
    while (i < edges) {
        ++revCnt[dst[i]];
        ++i;
    }

    // Prefix sums for reversed graph
    int* revPos = new int[vertices + 1];
    revPos[0] = 0;
    for (int v = 0; v < vertices; ++v) {
        revPos[v + 1] = revPos[v] + revCnt[v];
    }

    int* revAdj = new int[edges];
    int* curPos2 = new int[vertices];
    for (int v = 0; v < vertices; ++v) curPos2[v] = revPos[v];

    // Fill reversed adjacency (unrolled)
    i = 0;
    while (i + 3 < edges) {
        revAdj[curPos2[dst[i]]++] = src[i];
        revAdj[curPos2[dst[i + 1]]++] = src[i + 1];
        revAdj[curPos2[dst[i + 2]]++] = src[i + 2];
        revAdj[curPos2[dst[i + 3]]++] = src[i + 3];
        i += 4;
    }
    while (i < edges) {
        revAdj[curPos2[dst[i]]++] = src[i];
        ++i;
    }

    // ---------- Print a small portion of both graphs ----------
    std::cout << "Original adjacency (first 5 vertices):\n";
    for (int v = 0; v < 5; ++v) {
        std::cout << "  " << v << ": ";
        for (int p = outPos[v]; p < outPos[v + 1]; ++p) {
            std::cout << outAdj[p] << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\nReversed adjacency (first 5 vertices):\n";
    for (int v = 0; v < 5; ++v) {
        std::cout << "  " << v << ": ";
        for (int p = revPos[v]; p < revPos[v + 1]; ++p) {
            std::cout << revAdj[p] << " ";
        }
        std::cout << "\n";
    }

    // ---------- Cleanup ----------
    delete[] src;
    delete[] dst;
    delete[] outCnt;
    delete[] outPos;
    delete[] outAdj;
    delete[] curPos;
    delete[] revCnt;
    delete[] revPos;
    delete[] revAdj;
    delete[] curPos2;

    return 0;
}
