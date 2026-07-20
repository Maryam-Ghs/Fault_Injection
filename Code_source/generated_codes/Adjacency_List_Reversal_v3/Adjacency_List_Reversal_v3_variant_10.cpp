#include <iostream>
#include <vector>

using namespace std;

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: allocate an array of adjacency vectors on the heap
// ------------------------------------------------------------
vector<int>* allocateAdj(int vCount) {
    // new creates an array of vCount default‑constructed vectors
    vector<int>* adj = new vector<int>[vCount];
    return adj;
}

// ------------------------------------------------------------
// Helper: free the heap‑allocated adjacency array
// ------------------------------------------------------------
void freeAdj(vector<int>* adj) {
    delete[] adj;
}

// ------------------------------------------------------------
// Helper: build the forward adjacency list from edge arrays
// ------------------------------------------------------------
void buildForward(int vCount, int eCount, int* srcArr, int* dstArr, vector<int>* fAdj) {
    int i = 0;
    while (i < eCount) {
        // each edge is srcArr[i] -> dstArr[i]
        int from = srcArr[i];
        int to   = dstArr[i];

        // insert the destination into the source's list
        fAdj[from].push_back(to);

        // advance the index manually
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Helper: construct the reversed adjacency list
// ------------------------------------------------------------
void computeReverse(int vCount, int eCount, int* srcArr, int* dstArr, vector<int>* rAdj) {
    int j = 0;
    while (j < eCount) {
        int origFrom = srcArr[j];
        int origTo   = dstArr[j];

        // reverse direction: origTo -> origFrom
        rAdj[origTo].push_back(origFrom);

        j = j + 1;
    }
}

// ------------------------------------------------------------
// Helper: print an adjacency list with a title
// ------------------------------------------------------------
void displayAdj(char* title, int vCount, vector<int>* adj) {
    cout << title << endl;
    int v = 0;
    while (v < vCount) {
        cout << " vertex " << v << ": ";

        int k = 0;
        // size() returns size_t; cast to int for our loop variable
        int sz = (int)adj[v].size();
        while (k < sz) {
            cout << adj[v][k];
            if (k < sz - 1) {
                cout << " -> ";
            }
            k = k + 1;
        }
        cout << endl;
        v = v + 1;
    }
    cout << endl;
}

// ============================================================
// main: generate a larger safe graph, reverse it, and print both
// ============================================================
int main() {
    // ---------- 1. define graph size ----------
    int vertexCount = 1000;   // vertices are 0 … 999
    int edgeCount   = 2000;   // number of directed edges

    // ---------- 2. generate deterministic edge list ----------
    int* sourceArr = new int[edgeCount];
    int* destArr   = new int[edgeCount];
    for (int i = 0; i < edgeCount; ++i) {
        // Simple pattern ensuring valid vertices:
        // source cycles through vertices, destination is (source * 2 + 1) % vertexCount
        sourceArr[i] = i % vertexCount;
        destArr[i]   = (sourceArr[i] * 2 + 1) % vertexCount;
    }

    // ---------- 3. allocate adjacency structures ----------
    vector<int>* forwardAdj = allocateAdj(vertexCount);
    vector<int>* reverseAdj = allocateAdj(vertexCount);

    // ---------- 4. build forward adjacency ----------
    buildForward(vertexCount, edgeCount, sourceArr, destArr, forwardAdj);

    // ---------- 5. compute reversed adjacency ----------
    computeReverse(vertexCount, edgeCount, sourceArr, destArr, reverseAdj);

    // ---------- 6. display results ----------
    char titleOrig[] = "Original adjacency list:";
    char titleRev[]  = "Reversed adjacency list:";
    displayAdj(titleOrig, vertexCount, forwardAdj);
    displayAdj(titleRev,  vertexCount, reverseAdj);

    // ---------- 7. clean up heap memory ----------
    freeAdj(forwardAdj);
    freeAdj(reverseAdj);
    delete[] sourceArr;
    delete[] destArr;

    return 0;
}
