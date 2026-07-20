#include <bits/stdc++.h>

 /* LLM input variant 2: small-diverse */

class CircleArrangements {
public:
    int qty;          // number of circles
    float* rad;       // radii on heap

    CircleArrangements(int sz) : qty(sz) {
        rad = new float[qty];
    }

    ~CircleArrangements() {
        delete[] rad;
    }

    void fillFrom(const float* src) {
        for (int i = 0; i < qty; ++i) rad[i] = src[i];
    }

    int distinct() {
        std::sort(rad, rad + qty);
        int* mult = new int[qty];
        int uniq = 0;
        int pos = 0;
        while (pos < qty) {
            int nxt = pos + 1;
            while (nxt < qty && rad[nxt] == rad[pos]) ++nxt;
            mult[uniq++] = nxt - pos;
            pos = nxt;
        }
        int factAll = 1;
        for (int k = qty; k > 1; --k) factAll = factAll * k;
        int result = factAll;
        for (int u = 0; u < uniq; ++u) {
            int f = 1;
            for (int t = mult[u]; t > 1; --t) f = f * t;
            result = result / f;
        }
        delete[] mult;
        return result;
    }
};

int main() {
    const int testCount = 2;
    const int sizes[testCount] = {3, 4};

    const float dataFlat[] = {
        // test 0: 1, 2, 3
        1.0f, 2.0f, 3.0f,
        // test 1: 2, 2, 3, 4
        2.0f, 2.0f, 3.0f, 4.0f
    };
    const int offsets[testCount] = {0, 3};

    for (int t = 0; t < testCount; ++t) {
        int n = sizes[t];
        CircleArrangements solver(n);
        solver.fillFrom(dataFlat + offsets[t]);
        int ways = solver.distinct();
        std::cout << "Test " << t << ": ";
        for (int i = 0; i < n; ++i) std::cout << solver.rad[i] << (i + 1 == n ? "" : " ");
        std::cout << " -> distinct arrangements = " << ways << '\n';
    }
    return 0;
}
