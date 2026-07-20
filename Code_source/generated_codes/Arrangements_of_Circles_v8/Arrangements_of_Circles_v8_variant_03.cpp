#include <iostream>
#include <cstdlib>

/* LLM input variant 3: zeros-and-ones */

//------------------------------------------------------------
// Fill an integer stack array with random radii (1 .. 9)
//------------------------------------------------------------
void fillRadii(int rad[], int cnt) {
    for (int i = 0; i < cnt; ++i) rad[i] = (std::rand() % 9) + 1;
}

//------------------------------------------------------------
// Compute total sum
//------------------------------------------------------------
int totalSum(const int rad[], int cnt) {
    int acc = 0;
    for (int i = 0; i < cnt; ++i) acc += rad[i];
    return acc;
}

//------------------------------------------------------------
// Count subsets
//------------------------------------------------------------
int countFitSubsets(const int rad[], int cnt, int bound) {
    int fits = 0;
    int subsets = 1 << cnt;
    for (int mask = 0; mask < subsets; ++mask) {
        int curSum = 0;
        for (int pos = 0; pos < cnt; ++pos) {
            if (mask & (1 << pos)) {
                curSum += rad[pos];
                if (curSum > bound) break;
            }
        }
        if (curSum <= bound) ++fits;
    }
    return fits;
}

//------------------------------------------------------------
// Main driver – zeros and ones.
//------------------------------------------------------------
int main() {
    int circleCnt = 4;
    int radii[20] = {0, 1, 0, 1};

    int sumAll = totalSum(radii, circleCnt);
    int limit = sumAll / 2; // will be 1

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
