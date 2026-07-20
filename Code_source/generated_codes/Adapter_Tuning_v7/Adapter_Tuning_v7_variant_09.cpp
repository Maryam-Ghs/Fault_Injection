// LLM input variant 9: medium-deterministic-random
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
    // Generate 30 pseudo‑random looking Celsius values deterministically
    const int numVals = 30;
    source.rawVals.reserve(numVals);
    int seed = 12345; // fixed seed
    for (int i = 0; i < numVals; ++i) {
        // Simple linear congruential generator: X_{n+1} = (a*X_n + c) mod m
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        int val = (seed % 201) - 100; // range [-100, 100]
        source.rawVals.push_back(val);
    }

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
