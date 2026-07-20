#include <iostream>
#include <vector>

/* LLM input variant 1: minimal-boundary */

class AlphaProcessor {
public:
    // Process a batch of integer vectors and produce an integer result for each
    std::vector<int> run(const std::vector<std::vector<int>>& batch) {
        std::vector<int> out;
        // iterate over each test case
        for (int i = 0; i < (int)batch.size(); ++i) {
            const std::vector<int>& cur = batch[i];
            int acc = 0;                     // accumulator for sum of squares
            int idx = 0;
            // loop‑heavy: compute sum of squares
            while (idx < (int)cur.size()) {
                int val = cur[idx];
                acc += val * val;
                ++idx;
            }
            // reorder: multiply by length after sum is ready
            int res = acc * (int)cur.size();
            // another loop to add a linear term (e.g., sum of elements)
            int lin = 0;
            for (int j = 0; j < (int)cur.size(); ++j) {
                lin += cur[j];
            }
            // final result combines both parts
            res += lin;
            out.push_back(res);
        }
        return out;
    }
};

int main() {
    // deterministic minimal test vector
    std::vector<std::vector<int>> testData;
    std::vector<int> vecMin; vecMin.push_back(0);
    testData.push_back(vecMin);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
