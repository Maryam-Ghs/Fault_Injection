#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------------
// Helper: fill a vector with pseudo‑random floats in [low, high]
// ------------------------------------------------------------------
std::vector<float> createRandomVector(int length, float low, float high)
{
    std::vector<float> result;
    result.reserve(length);

    int index = 0;
    while (index < length)
    {
        // generate a random integer, scale to [0,1], then to [low,high]
        float raw = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        float value = low + raw * (high - low);
        result.push_back(value);
        ++index;
    }
    return result;
}

// ------------------------------------------------------------------
// Helper: compute the adapter (single scaling factor) that best maps
//         source values onto target values in the least‑squares sense.
//         factor = Σ(source_i * target_i) / Σ(source_i * source_i)
// ------------------------------------------------------------------
float deriveAdapter(std::vector<float>& source, std::vector<float>& target)
{
    // accumulate Σ(source_i * target_i)
    float numerator = 0.0f;
    int pos = 0;
    while (pos < static_cast<int>(source.size()))
    {
        float prod = source[pos] * target[pos];
        numerator = numerator + prod;          // split into temporary var
        ++pos;
    }

    // accumulate Σ(source_i * source_i)
    float denominator = 0.0f;
    int idx = 0;
    while (idx < static_cast<int>(source.size()))
    {
        float sq = source[idx] * source[idx];
        denominator = denominator + sq;        // split into temporary var
        ++idx;
    }

    // avoid division by zero
    if (denominator == 0.0f)
        return 0.0f;

    // compute the scaling factor
    float factor = numerator / denominator;
    return factor;
}

// ------------------------------------------------------------------
// Helper: apply the adapter to every element of a vector
// ------------------------------------------------------------------
std::vector<float> applyAdapter(std::vector<float>& source, float factor)
{
    std::vector<float> adapted;
    adapted.reserve(source.size());

    int i = 0;
    while (i < static_cast<int>(source.size()))
    {
        float scaled = source[i] * factor;
        adapted.push_back(scaled);
        ++i;
    }
    return adapted;
}

// ------------------------------------------------------------------
// Helper: compute mean‑squared error between two vectors
// ------------------------------------------------------------------
float computeMSE(std::vector<float>& a, std::vector<float>& b)
{
    float sumSquares = 0.0f;
    int k = 0;
    while (k < static_cast<int>(a.size()))
    {
        float diff = a[k] - b[k];
        float sq   = diff * diff;
        sumSquares = sumSquares + sq;           // split into temporary var
        ++k;
    }

    // divide by number of elements
    float mse = sumSquares / static_cast<float>(a.size());
    return mse;
}

// ------------------------------------------------------------------
// Main driver
// ------------------------------------------------------------------
int main()
{
    // deterministic seed for reproducibility
    std::srand(12345);

    // --------------------------------------------------------------
    // 1. generate a sparse source array of larger size
    // --------------------------------------------------------------
    int arraySize = 100;                    // larger array for sparsity
    float lowBound = -10.0f;
    float highBound = 10.0f;
    std::vector<float> srcVals = createRandomVector(arraySize, lowBound, highBound);

    // make the vector sparse: only every 7th element retains its random value,
    // others are set to zero (clustered non‑zero entries)
    for (int i = 0; i < arraySize; ++i)
    {
        if (i % 7 != 0)
            srcVals[i] = 0.0f;
    }

    // --------------------------------------------------------------
    // 2. define a hidden “true” adapter and create a target array
    // --------------------------------------------------------------
    float trueFactor = 2.5f;                // hidden scaling factor
    std::vector<float> tgtVals;
    tgtVals.reserve(arraySize);

    int j = 0;
    while (j < arraySize)
    {
        // add a small noise term in [-0.5,0.5]
        float noiseRaw = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        float noise = -0.5f + noiseRaw * 1.0f;

        float transformed = srcVals[j] * trueFactor + noise;
        tgtVals.push_back(transformed);
        ++j;
    }

    // --------------------------------------------------------------
    // 3. estimate the adapter from the generated data
    // --------------------------------------------------------------
    float estimatedFactor = deriveAdapter(srcVals, tgtVals);

    // --------------------------------------------------------------
    // 4. apply the estimated adapter to the source and evaluate error
    // --------------------------------------------------------------
    std::vector<float> adaptedVals = applyAdapter(srcVals, estimatedFactor);
    float mse = computeMSE(adaptedVals, tgtVals);

    // --------------------------------------------------------------
    // 5. print the results
    // --------------------------------------------------------------
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "True adapter          : " << trueFactor << "\n";
    std::cout << "Estimated adapter     : " << estimatedFactor << "\n";
    std::cout << "Mean‑squared error    : " << mse << "\n";

    // optional: display a few sample pairs
    std::cout << "\nSample comparison (source → target → adapted):\n";
    int displayCount = 5;
    int d = 0;
    while (d < displayCount && d < arraySize)
    {
        std::cout << "  " << srcVals[d]
                  << " → " << tgtVals[d]
                  << " → " << adaptedVals[d] << "\n";
        ++d;
    }

    return 0;
}
