#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 5: duplicate-heavy */
int main() {
    // ---------- generate graph where many duplicate attempts occur ----------
    int vertexCount = 4;
    int edgeCount   = 6;  // full edge set, many attempts will be duplicates
    int maxRandVal  = vertexCount - 1;

    vector< vector<int> > adjacency(vertexCount);
    srand(1);

    int generated = 0;
    while (generated < edgeCount) {
        // deliberately using small range to increase duplicate chance
        int a = rand() % 10;   // limited range
        int b = rand() % 10;
        int u = a % vertexCount;
        int v = b % vertexCount;

        if (u == v) continue;
        bool duplicate = false;
        for (int x : adjacency[u]) if (x == v) { duplicate = true; break; }
        if (duplicate) continue;

        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
        ++generated;
    }

    // ---------- iterative Tarjan for articulation points ----------
    vector<int> order(vertexCount, -1);
    vector<int> lowLink(vertexCount, -1);
    vector<int> parentNode(vertexCount, -1);
    vector<int> childNum(vertexCount, 0);
    vector<int> articulationFlag(vertexCount, 0);
    int timeCounter = 0;

    struct Frame { int node; int iterIdx; };
    vector<Frame> stackFrames;

    for (int start = 0; start < vertexCount; ++start) {
        if (order[start] != -1) continue;
        stackFrames.push_back({start, 0});
        parentNode[start] = -1;

        while (!stackFrames.empty()) {
            Frame &cur = stackFrames.back();
            int v = cur.node;
            if (order[v] == -1) order[v] = lowLink[v] = timeCounter++;

            if (cur.iterIdx < (int)adjacency[v].size()) {
                int nxt = adjacency[v][cur.iterIdx++];
                if (order[nxt] == -1) {
                    parentNode[nxt] = v;
                    childNum[v]++;
                    stackFrames.push_back({nxt, 0});
                } else if (nxt != parentNode[v]) {
                    if (lowLink[v] > order[nxt]) lowLink[v] = order[nxt];
                }
            } else {
                if (parentNode[v] != -1) {
                    int p = parentNode[v];
                    if (lowLink[p] > lowLink[v]) lowLink[p] = lowLink[v];
                    if (lowLink[v] >= order[p]) articulationFlag[p] = 1;
                } else {
                    if (childNum[v] > 1) articulationFlag[v] = 1;
                }
                stackFrames.pop_back();
            }
        }
    }

    cout << "Articulation points (total " << vertexCount << " vertices):\n";
    int artCount = 0;
    for (int i = 0; i < vertexCount; ++i) {
        if (articulationFlag[i]) { cout << i << ' '; ++artCount; }
    }
    if (artCount == 0) cout << "None";
    cout << "\nTotal articulation points found: " << artCount << '\n';
    return 0;
}
