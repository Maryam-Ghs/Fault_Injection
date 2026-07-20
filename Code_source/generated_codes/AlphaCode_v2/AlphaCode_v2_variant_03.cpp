#include <iostream>
#include <vector>

/* LLM input variant 3: zeros-and-ones */

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
    // deterministic test vectors emphasizing zeros, ones, and empty case
    std::vector<std::vector<int>> testData;

    std::vector<int> vecEmpty;                     // empty vector

    std::vector<int> vecZeros; 
    vecZeros.push_back(0); vecZeros.push_back(0); vecZeros.push_back(0); vecZeros.push_back(0);

    std::vector<int> vecOnes; 
    vecOnes.push_back(1); vecOnes.push_back(1); vecOnes.push_back(1); vecOnes.push_back(1); vecOnes.push_back(1);

    std::vector<int> vecMixed; 
    vecMixed.push_back(0); vecMixed.push_back(1); vecMixed.push_back(0); vecMixed.push_back(1); vecMixed.push_back(0);

    testData.push_back(vecEmpty);
    testData.push_back(vecZeros);
    testData.push_back(vecOnes);
    testData.push_back(vecMixed);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
