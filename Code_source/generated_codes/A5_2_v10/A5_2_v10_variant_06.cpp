/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <vector>

class A5_2 {
public:
    // Generate a deterministic ordered-structured data set
    void generateData() {
        rawData.clear();
        // Sorted ascending sequence with symmetric negative and positive values
        rawData.push_back(-10);
        rawData.push_back(-5);
        rawData.push_back(-2);
        rawData.push_back(0);
        rawData.push_back(1);
        rawData.push_back(3);
        rawData.push_back(7);
        rawData.push_back(12);
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
