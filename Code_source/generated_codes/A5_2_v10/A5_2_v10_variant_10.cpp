#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

class A5_2 {
public:
    // Generate a larger predefined data set while staying within safe integer limits
    void generateData() {
        rawData.clear();
        // Create 1000 elements ranging from -70 to +70 (inclusive) to avoid overflow in val^5 computation
        for (int i = 0; i < 1000; ++i) {
            int val = (i % 141) - 70; // yields values -70 .. 70
            rawData.push_back(val);
        }
    }

    // Process each element: compute (value^5) / 2 using fused integer arithmetic
    void computeResults() {
        processed.clear();
        std::size_t idx = 0;
        while (idx < rawData.size()) {
            int val = rawData[idx];
            // fused expression: ((val*val)*(val*val)*val) / 2
            int fifth = ((val * val) * (val * val) * val) / 2;
            processed.push_back(fifth);
            ++idx;
        }
    }

    // Output the original data and the computed results
    void display() const {
        std::size_t i = 0;
        while (i < rawData.size()) {
            std::cout << "Input[" << i << "] = " << rawData[i]
                      << "  =>  Output[" << i << "] = " << processed[i] << '\n';
            ++i;
        }
    }

private:
    std::vector<int> rawData;      // holds the generated inputs
    std::vector<int> processed;    // holds the computed (value^5)/2 results
};

int main() {
    // Version #10 of the A5/2 implementation
    A5_2 solver;
    solver.generateData();   // internal input creation
    solver.computeResults(); // loop‑heavy iterative processing
    solver.display();        // print results
    return 0;
}
