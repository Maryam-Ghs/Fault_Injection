#include <iostream>
#include <vector>

/* LLM input variant 2: small-diverse */

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
    // deterministic test vectors – varied small dataset
    std::vector<std::vector<int>> testData;
    std::vector<int> vec1; vec1.push_back(10); vec1.push_back(-2); vec1.push_back(5);
    std::vector<int> vec2; vec2.push_back(0); vec2.push_back(0); vec2.push_back(0); vec2.push_back(0);
    std::vector<int> vec3; vec3.push_back(7);
    std::vector<int> vec4; vec4.push_back(3); vec4.push_back(4); vec4.push_back(5); vec4.push_back(6); vec4.push_back(7);
    testData.push_back(vec1);
    testData.push_back(vec2);
    testData.push_back(vec3);
    testData.push_back(vec4);

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
