#include <iostream>
#include <vector>

/* LLM input variant 5: duplicate-heavy */

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
    // deterministic test vectors with heavy duplicate patterns
    std::vector<std::vector<int>> testData;
    std::vector<int> vecA = {5, 5, 5, 5};
    std::vector<int> vecB = {2, 2, 2, 2, 2, 2};
    std::vector<int> vecC = {0, 0, 0};
    std::vector<int> vecD = {7, 7, 7, 7, 7};
    std::vector<int> vecE = {5, 5, 5, 5}; // duplicate of vecA
    testData.push_back(vecA);
    testData.push_back(vecB);
    testData.push_back(vecC);
    testData.push_back(vecD);
    testData.push_back(vecE);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
