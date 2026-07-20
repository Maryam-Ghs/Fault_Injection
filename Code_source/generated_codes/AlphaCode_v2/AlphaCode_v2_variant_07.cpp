#include <iostream>
#include <vector>

/* LLM input variant 7: reverse-adversarial */

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
    // deterministic adversarial test vectors
    std::vector<std::vector<int>> testData;

    // Vector 1: descending integers from 1000 down to 1 (worst‑case for many algorithms)
    std::vector<int> vecDesc;
    for (int v = 1000; v >= 1; --v) {
        vecDesc.push_back(v);
    }
    testData.push_back(vecDesc);

    // Vector 2: ascending integers from 1 up to 1000 (reverse of the first)
    std::vector<int> vecAsc;
    for (int v = 1; v <= 1000; ++v) {
        vecAsc.push_back(v);
    }
    testData.push_back(vecAsc);

    // Vector 3: alternating large positive and negative values to stress linear term
    std::vector<int> vecAlt;
    for (int i = 0; i < 500; ++i) {
        vecAlt.push_back(30000);   // moderate positive
        vecAlt.push_back(-30000);  // moderate negative
    }
    testData.push_back(vecAlt);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
