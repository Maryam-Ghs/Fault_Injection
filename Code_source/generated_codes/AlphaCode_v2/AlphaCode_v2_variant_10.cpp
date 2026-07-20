#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

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
    // generate a large but safe deterministic test dataset
    const int numVectors = 1000;        // number of test vectors
    const int vectorSize = 200;        // size of each vector (kept modest to avoid overflow)
    std::vector<std::vector<int>> testData;
    testData.reserve(numVectors);

    for (int v = 0; v < numVectors; ++v) {
        std::vector<int> vec;
        vec.reserve(vectorSize);
        // fill with a deterministic pattern within [-50, 49] to keep squares small
        for (int i = 0; i < vectorSize; ++i) {
            int val = ((v + i) % 100) - 50; // range -50..49
            vec.push_back(val);
        }
        testData.push_back(std::move(vec));
    }

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
