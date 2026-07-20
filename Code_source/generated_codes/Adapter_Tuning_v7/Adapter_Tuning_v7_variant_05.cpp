/* LLM input variant 5: duplicate-heavy */
// Adapter Tuning – version #7
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
    source.rawVals = {
        -40, -40, -40,    // repeated extreme low
        0, 0, 0, 0,        // multiple zeros
        20, 20, 20, 20,   // repeated moderate value
        37, 37, 37,        // repeated typical human body temp
        100, 100, 100, 100, 100 // many repeats of boiling point
    };

    // ---------- adapter instance ----------
    CelsiusToFahrenheit adapt;
    adapt.origin = &source;

    // ---------- conversion and output ----------
    std::vector<float> convVals;
    int idx = 0;
    int total = static_cast<int>(source.rawVals.size());
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
