#include <iostream>
#include <vector>

/* LLM input variant 8: sparse-skewed */

class A5_2 {
public:
    // Generate a larger sparse, skewed data set
    void generateData() {
        // Create a vector of 1000 elements initialized to 0 (mostly empty)
        rawData.clear();
        rawData.resize(1000, 0);

        // Insert a few non‑zero values to create imbalance and skew
        rawData[0] = 1;          // small positive
        rawData[250] = -2;       // small negative
        rawData[500] = 3;        // another small positive
        rawData[750] = -5;       // another small negative
        rawData[999] = 80;       // large positive outlier (still within safe range for int^5)
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
