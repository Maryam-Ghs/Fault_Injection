#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 2: small-diverse */
int main() {
    /* ---------- predefined polygon (simple, CCW) ---------- */
    int vcnt = 6;
    int X[10] = {0, 4, 5, 3, 1, 0};
    int Y[10] = {0, 0, 2, 4, 3, 1};

    /* ---------- ear‑clipping triangulation (stack arrays) ---------- */
    int cut[10] = {0};
    int tri[20][3];
    int tnum = 0;
    int left = vcnt;

    while (left > 3) {
        int i = 0;
        while (i < vcnt) {
            if (cut[i]) { i++; continue; }

            int pre = (i - 1 + vcnt) % vcnt;
            while (cut[pre]) pre = (pre - 1 + vcnt) % vcnt;
            int nxt = (i + 1) % vcnt;
            while (cut[nxt]) nxt = (nxt + 1) % vcnt;

            int cr = (X[i] - X[pre]) * (Y[nxt] - Y[pre]) -
                     (Y[i] - Y[pre]) * (X[nxt] - X[pre]);
            bool conv = cr > 0;
            bool ear = conv;

            int j = 0;
            while (j < vcnt && ear) {
                if (!cut[j] && j != pre && j != i && j != nxt) {
                    int c1 = (X[i] - X[pre]) * (Y[j] - Y[pre]) -
                             (Y[i] - Y[pre]) * (X[j] - X[pre]);
                    int c2 = (X[nxt] - X[i]) * (Y[j] - Y[i]) -
                             (Y[nxt] - Y[i]) * (X[j] - X[i]);
                    int c3 = (X[pre] - X[nxt]) * (Y[j] - Y[nxt]) -
                             (Y[pre] - Y[nxt]) * (X[j] - X[nxt]);
                    ear = !( (c1 >= 0 && c2 >= 0 && c3 >= 0) ||
                             (c1 <= 0 && c2 <= 0 && c3 <= 0) );
                }
                j++;
            }

            if (ear) {
                tri[tnum][0] = pre; tri[tnum][1] = i; tri[tnum][2] = nxt;
                tnum++; cut[i] = 1; left--; break;
            }
            i++;
        }
    }

    int a = 0, b = 1, c = 2;
    while (cut[a]) a++;
    while (cut[b] || b == a) b++;
    while (cut[c] || c == a || c == b) c++;
    tri[tnum][0] = a; tri[tnum][1] = b; tri[tnum][2] = c; tnum++;

    int guards = vcnt / 3;

    cout << "Polygon vertices (CCW):\n";
    for (int k = 0; k < vcnt; k++)
        cout << '(' << X[k] << ',' << Y[k] << ") ";
    cout << "\nTriangulation (vertex indices):\n";
    for (int k = 0; k < tnum; k++)
        cout << '[' << tri[k][0] << ',' << tri[k][1] << ',' << tri[k][2] << "] ";
    cout << "\nMinimum guards needed (theorem ⌊n/3⌋): " << guards << '\n';
    return 0;
}
