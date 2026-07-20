#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

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

// Simple deterministic pseudo‑random generator (linear congruential)
static int lcg_next(int& state) {
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return state;
}

// Generate a vector of given length with deterministic values based on a seed
static std::vector<int> generate_vector(int length, int seed) {
    std::vector<int> v;
    v.reserve(length);
    int state = seed;
    for (int i = 0; i < length; ++i) {
        int rnd = lcg_next(state);
        // map to range [-30, 30]
        int val = (rnd % 61) - 30;
        v.push_back(val);
    }
    return v;
}

int main() {
    // deterministic test vectors (medium‑size, diverse)
    std::vector<std::vector<int>> testData;
    testData.push_back(generate_vector(5,  1));
    testData.push_back(generate_vector(7,  2));
    testData.push_back(generate_vector(9,  3));
    testData.push_back(generate_vector(6,  4));
    testData.push_back(generate_vector(8,  5));
    testData.push_back(generate_vector(10, 6));
    testData.push_back(generate_vector(4,  7));
    testData.push_back(generate_vector(12, 8));

    AlphaProcessor solver;
    std::vector<int> answers = solver.run(testData);

    // print results
    for (int k = 0; k < (int)answers.size(); ++k) {
        std::cout << "Result[" << k << "] = " << answers[k] << std::endl;
    }
    return 0;
}
