#include <iostream>
#include <cstdlib>

/* LLM input variant 7: reverse-adversarial */

//------------------------------------------------------------
void fillRadii(int rad[], int cnt) {
    for (int i = 0; i < cnt; ++i) rad[i] = (std::rand() % 9) + 1;
}

//------------------------------------------------------------
int totalSum(const int rad[], int cnt) {
    int acc = 0;
    for (int i = 0; i < cnt; ++i) acc += rad[i];
    return acc;
}

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
int main() {
    int circleCnt = 5;
    int radii[20] = {9, 8, 7, 6, 5};

    int sumAll = totalSum(radii, circleCnt);
    int lowBound = sumAll / 2; // 17
    int limit = lowBound + 10; // 27 – pushes towards the upper side

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
