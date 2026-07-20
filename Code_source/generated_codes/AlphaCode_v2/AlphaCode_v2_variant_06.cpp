#include <iostream>
#include <vector>

/* LLM input variant 6: ordered-structured */

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
    // deterministic, highly structured test vectors
    std::vector<std::vector<int>> testData;

    // Arithmetic progression 0..4
    std::vector<int> vecA = {0, 1, 2, 3, 4};

    // Negative ascending sequence -5..0
    std::vector<int> vecB = {-5, -4, -3, -2, -1, 0};

    // Symmetric palindromic sequence
    std::vector<int> vecC = {10, 8, 6, 4, 2, 0, 2, 4, 6, 8, 10};

    testData.push_back(vecA);
    testData.push_back(vecB);
    testData.push_back(vecC);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
