#include <bits/stdc++.h>

 /* LLM input variant 1: minimal-boundary */

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

    // copy from a static source array
    void fillFrom(const float* src) {
        for (int i = 0; i < qty; ++i) rad[i] = src[i];
    }

    // compute distinct linear arrangements (permutations) ignoring identical radii
    int distinct() {
        // 1) bring equal radii together
        std::sort(rad, rad + qty);

        // 2) build multiplicity list on heap
        int* mult = new int[qty];
        int uniq = 0;
        int pos = 0;
        while (pos < qty) {
            int nxt = pos + 1;
            while (nxt < qty && rad[nxt] == rad[pos]) ++nxt;
            mult[uniq++] = nxt - pos;          // store count of this radius
            pos = nxt;
        }

        // 3) factorial of total count (reordered multiplication)
        int factAll = 1;
        for (int k = qty; k > 1; --k) factAll = factAll * k;   // descending order

        // 4) divide by factorials of each multiplicity (reordered division)
        int result = factAll;
        for (int u = 0; u < uniq; ++u) {
            int f = 1;
            for (int t = mult[u]; t > 1; --t) f = f * t;       // descending order
            result = result / f;
        }

        delete[] mult;
        return result;
    }
};

int main() {
    // deterministic test vectors (qty, radii)
    const int testCount = 1;
    const int sizes[testCount] = {1};

    // static source data stored flat; each sub‑array starts at offsets[ i ]
    const float dataFlat[] = {
        1.0f
    };
    const int offsets[testCount] = {0};

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
