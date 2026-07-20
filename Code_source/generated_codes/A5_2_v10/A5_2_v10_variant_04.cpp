#include <iostream>
#include <vector>

/* LLM input variant 4: signed-extremes */
class A5_2 {
public:
    // Generate a small predefined data set
    void generateData() {
        // Example data: a mix of negative, zero, and positive integers within safe range
        rawData.clear();
        rawData.push_back(0);
        rawData.push_back(7);
        rawData.push_back(-8);
        rawData.push_back(15);
        rawData.push_back(-20);
        rawData.push_back(33);
        rawData.push_back(-44);
        rawData.push_back(55);
        rawData.push_back(-66);
        rawData.push_back(77);
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
