#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

/* -------------------------------------------------------------
   ALIGN version #7
   -------------------------------------------------------------
   This program creates two random integer arrays and finds the
   offset that aligns them with the smallest total absolute
   difference.  The implementation follows the required style:
   - Verbose step‑by‑step processing
   - Reordered arithmetic where possible
   - std::vector for memory handling
   - All logic encapsulated in a class
   - Only int and float types are used
   ------------------------------------------------------------- */

class AlignProcessor {
public:
    // Public interface
    void run() {
        generateData();
        displayData();
        computeBestAlignment();
        reportResult();
    }

private:
    // -----------------------------------------------------------------
    // Member variables (no const, no unsigned, no double, no long)
    // -----------------------------------------------------------------
    vector<int> vA;          // first random array
    vector<int> vB;          // second random array
    int lenA;                // length of vA
    int lenB;                // length of vB
    int bestShift;           // offset that gives minimal score
    int minimalScore;        // minimal total absolute difference

    // -----------------------------------------------------------------
    // Step 1: generate two medium‑sized structured arrays
    // -----------------------------------------------------------------
    void generateData() {
        // Use deterministic, highly structured data (sorted, symmetric)

        // Fixed lengths within the original range (8‑15)
        lenA = 12;   // example length
        lenB = 10;   // example length

        // Fill vA with an ascending sequence of even numbers
        vA.clear();
        for (int i = 0; i < lenA; ++i) {
            vA.push_back(i * 2);          // 0,2,4,...
        }

        // Fill vB with a descending sequence that mirrors part of vA
        vB.clear();
        for (int i = 0; i < lenB; ++i) {
            vB.push_back((lenB - i - 1) * 3); // e.g., 27,24,21,...
        }
    }

    // -----------------------------------------------------------------
    // Step 2: display generated arrays
    // -----------------------------------------------------------------
    void displayData() {
        cout << "Array A (size " << lenA << "): ";
        for (size_t p = 0; p < vA.size(); ++p) {
            cout << vA[p] << " ";
        }
        cout << endl;

        cout << "Array B (size " << lenB << "): ";
        for (size_t p = 0; p < vB.size(); ++p) {
            cout << vB[p] << " ";
        }
        cout << endl;
    }

    // -----------------------------------------------------------------
    // Step 3: compute the best alignment offset
    // -----------------------------------------------------------------
    void computeBestAlignment() {
        // Offsets range from -(lenB-1) to (lenA-1)
        int lowShift = -lenB + 1;
        int highShift = lenA - 1;

        // Initialise best values with a large sentinel
        bestShift = lowShift;
        minimalScore = 2147483647;          // max int value

        int curShift = lowShift;
        while (curShift <= highShift) {
            // Compute score for this shift
            int curScore = 0;
            int idxA = 0;
            while (idxA < lenA) {
                int idxB = idxA - curShift;     // reordered arithmetic
                // Check overlapping condition
                if (idxB >= 0 && idxB < lenB) {
                    int diff = vA[idxA] - vB[idxB];
                    int absDiff = diff;
                    // Reorder: make positive without std::abs
                    if (absDiff < 0) {
                        absDiff = -absDiff;
                    }
                    curScore = curScore + absDiff;   // addition after absolute
                }
                idxA = idxA + 1;
            }

            // Update best if this score is lower
            if (curScore < minimalScore) {
                minimalScore = curScore;
                bestShift = curShift;
            }

            curShift = curShift + 1;   // move to next offset
        }
    }

    // -----------------------------------------------------------------
    // Step 4: report the alignment result
    // -----------------------------------------------------------------
    void reportResult() {
        cout << "Best offset (shift of B relative to A): " << bestShift << endl;
        cout << "Minimal total absolute difference: " << minimalScore << endl;

        // Show the aligned view for illustration
        cout << "Aligned view (A on top, B shifted):" << endl;
        // Determine the visual width
        int visualWidth = max(lenA, lenB + bestShift);
        if (bestShift < 0) {
            visualWidth = max(lenA - bestShift, lenB);
        }

        // Print A with spaces for alignment
        for (int col = 0; col < visualWidth; ++col) {
            int aIdx = col;
            if (aIdx >= 0 && aIdx < lenA) {
                cout << setw(3) << vA[aIdx];
            } else {
                cout << "   ";
            }
        }
        cout << endl;

        // Print B with shift
        for (int col = 0; col < visualWidth; ++col) {
            int bIdx = col - bestShift;
            if (bIdx >= 0 && bIdx < lenB) {
                cout << setw(3) << vB[bIdx];
            } else {
                cout << "   ";
            }
        }
        cout << endl;
    }
};

int main() {
    AlignProcessor processor;
    processor.run();
    return 0;
}
