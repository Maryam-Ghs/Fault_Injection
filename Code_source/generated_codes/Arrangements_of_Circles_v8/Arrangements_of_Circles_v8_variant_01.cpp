#include <iostream>
#include <cstdlib>

/* LLM input variant 1: minimal-boundary */

//------------------------------------------------------------
// Fill an integer stack array with random radii (1 .. 9)
//------------------------------------------------------------
void fillRadii(int rad[], int cnt) {
    for (int i = 0; i < cnt; ++i) {
        rad[i] = (std::rand() % 9) + 1;          // radius between 1 and 9
    }
}

//------------------------------------------------------------
// Compute the total sum of the first 'cnt' radii
//------------------------------------------------------------
int totalSum(const int rad[], int cnt) {
    int acc = 0;
    for (int i = 0; i < cnt; ++i) {
        acc += rad[i];
    }
    return acc;
}

//------------------------------------------------------------
// Count how many subsets of the radii have a sum not larger
// than the given bound.  Simple enumeration (2^cnt) – suitable
// for modest cnt (≤ 20).
//------------------------------------------------------------
int countFitSubsets(const int rad[], int cnt, int bound) {
    int fits = 0;
    int subsets = 1 << cnt;                     // 2^cnt possibilities

    for (int mask = 0; mask < subsets; ++mask) {
        int curSum = 0;
        // iterate bits of mask
        for (int pos = 0; pos < cnt; ++pos) {
            // if the pos‑th bit is set, include rad[pos]
            if (mask & (1 << pos)) {
                curSum += rad[pos];
                // early break if already over the bound
                if (curSum > bound) break;
            }
        }
        if (curSum <= bound) ++fits;
    }
    return fits;
}

//------------------------------------------------------------
// Main driver – deterministic minimal input.
//------------------------------------------------------------
int main() {
    // minimal non‑trivial case: 1 circle with radius 1
    int circleCnt = 1;
    int radii[20] = {1};

    int sumAll = totalSum(radii, circleCnt);
    int limit  = sumAll / 2;                     // smallest valid bound (floor)

    int goodSets = countFitSubsets(radii, circleCnt, limit);

    std::cout << "Number of circles : " << circleCnt << '\n';
    std::cout << "Radii : ";
    for (int i = 0; i < circleCnt; ++i) {
        std::cout << radii[i] << (i + 1 == circleCnt ? '\n' : ' ');
    }
    std::cout << "Bound (max total radius) : " << limit << '\n';
    std::cout << "Subsets whose total radius ≤ bound : " << goodSets << '\n';
    return 0;
}
