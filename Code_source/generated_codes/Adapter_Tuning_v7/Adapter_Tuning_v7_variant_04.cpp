// Adapter Tuning – version #7
/* LLM input variant 4: signed-extremes */
#include <iostream>
#include <vector>

int main() {
    // ---------- legacy source (Celsius integers) ----------
    struct LegacySource {
        std::vector<int> rawVals;
        int fetch(int pos) {
            return rawVals[pos];
        }
    };

    // ---------- adapter to Fahrenheit (float) ----------
    struct CelsiusToFahrenheit {
        LegacySource* origin;
        float retrieve(int idx) {
            int celsius = origin->fetch(idx);
            // Reordered arithmetic: (celsius * 9) / 5 + 32
            float step1 = (float)celsius * 9;
            float step2 = step1 / 5;
            float result = step2 + 32;
            return result;
        }
    };

    // ---------- deterministic test vectors ----------
    LegacySource source;
    source.rawVals = { -273, -100, 0, 150, 300, 500 };   // Celsius values with mixed signs

    // ---------- adapter instance ----------
    CelsiusToFahrenheit adapt;
    adapt.origin = &source;

    // ---------- conversion and output ----------
    std::vector<float> convVals;
    int idx = 0;
    int total = source.rawVals.size();
    while (idx < total) {
        float f = adapt.retrieve(idx);
        convVals.push_back(f);
        ++idx;
    }

    // ---------- display ----------
    std::cout << "Celsius -> Fahrenheit conversion (adapter):\n";
    idx = 0;
    for (int i = 0; i < total; ++i) {
        std::cout << "  " << source.rawVals[i] << " C => " << convVals[i] << " F\n";
    }
    return 0;
}
