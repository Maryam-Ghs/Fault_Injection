#include <iostream>
#include <vector>

/* LLM input variant 8: sparse-skewed */

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
    // deterministic sparse and skewed test vectors
    std::vector<std::vector<int>> testData;

    // vecA: length 100, mostly zeros with a few scattered non‑zeros
    std::vector<int> vecA(100, 0);
    vecA[10] = 3;
    vecA[50] = -2;
    vecA[90] = 5;

    // vecB: very short vector with a single non‑zero at the end
    std::vector<int> vecB = {0, 0, 10};

    // vecC: length 200, zeros except a dense cluster from index 100 to 109
    std::vector<int> vecC(200, 0);
    for (int i = 0; i < 10; ++i) {
        vecC[100 + i] = i + 1; // values 1..10
    }

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
