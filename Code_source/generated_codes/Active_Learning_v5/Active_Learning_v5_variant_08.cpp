#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Helper: train a perceptron on a given set of indices
// ------------------------------------------------------------
void train_on_set(float* weight, float* features, int* labels,
                  int* idxSet, int setSize, int dim)
{
    int epoch = 0;
    while (epoch < 10)                     // fixed number of epochs
    {
        int i = 0;
        while (i < setSize)                // manual loop (no for)
        {
            int cur = idxSet[i];           // index of the sample
            // ---- dot product (manual unroll, reordered ops) ----
            float prod0 = features[cur * dim] * weight[0];
            float prod1 = features[cur * dim + 1] * weight[1];
            float dot   = prod0 + prod1;    // reorder: sum after mul
            // ---- prediction ----
            int pred = (dot >= 0) ? 1 : -1;
            // ---- update if mis‑classified ----
            if (pred != labels[cur])
            {
                // factor = learning_rate * label
                float factor = 0.1f * static_cast<float>(labels[cur]);
                // manual unroll of weight update
                weight[0] = weight[0] + factor * features[cur * dim];
                weight[1] = weight[1] + factor * features[cur * dim + 1];
            }
            i = i + 1;                     // manual increment
        }
        epoch = epoch + 1;
    }
}

// ------------------------------------------------------------
// Helper: find the most uncertain (smallest |margin|) sample
// ------------------------------------------------------------
int find_query(int* poolIdx, int poolSize,
               float* weight, float* features, int dim)
{
    int bestIdx = -1;
    float bestVal = 1e30f;                 // large initial value
    int i = 0;
    while (i < poolSize)
    {
        int cur = poolIdx[i];
        // ---- margin (manual unroll, reordered ops) ----
        float m0 = weight[0] * features[cur * dim];
        float m1 = weight[1] * features[cur * dim + 1];
        float margin = m0 + m1;
        float absMargin = std::fabs(margin);
        if (absMargin < bestVal)
        {
            bestVal = absMargin;
            bestIdx = i;                   // store position inside poolIdx
        }
        i = i + 1;
    }
    return bestIdx;
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    // --------------------------------------------------------
    // deterministic test vectors (2‑D) and ground truth
    // --------------------------------------------------------
    const int DIM = 2;                     // dimensionality (fixed)
    const int TOTAL = 8;                   // all points (labeled + pool)
    const int INITIAL = 1;                 // initially labelled (sparse start)
    const int POOL = TOTAL - INITIAL;      // pool size
    const int ACTIVE_ITER = 3;             // active learning rounds

    // heap allocation for features and labels
    float* data = new float[TOTAL * DIM];
    int*   truth = new int[TOTAL];

    // skewed, sparse, and imbalanced points
    //  x0   x1   label
    //  0.0  0.0   -1   (mostly empty)
    //  0.0  0.0   -1
    //  0.0  0.0   -1
    //  0.0  0.0   -1
    //  5.0  5.0   +1   (clustered positive)
    // -5.0 -5.0   -1   (outlier negative)
    //  0.0  0.0   -1
    // 10.0 10.0   +1   (another positive)
    data[0] =  0.0f; data[1] =  0.0f; truth[0] = -1;
    data[2] =  0.0f; data[3] =  0.0f; truth[1] = -1;
    data[4] =  0.0f; data[5] =  0.0f; truth[2] = -1;
    data[6] =  0.0f; data[7] =  0.0f; truth[3] = -1;
    data[8] =  5.0f; data[9] =  5.0f; truth[4] =  1;
    data[10]= -5.0f; data[11]= -5.0f; truth[5] = -1;
    data[12]=  0.0f; data[13]=  0.0f; truth[6] = -1;
    data[14]= 10.0f; data[15]= 10.0f; truth[7] =  1;

    // --------------------------------------------------------
    // bookkeeping arrays
    // --------------------------------------------------------
    int* labeledIdx = new int[TOTAL];
    int* poolIdx    = new int[POOL];
    int  labCount = 0;
    int  poolCount = 0;

    // initially labelled: first INITIAL points
    int i = 0;
    while (i < INITIAL)
    {
        labeledIdx[labCount] = i;
        labCount = labCount + 1;
        i = i + 1;
    }
    // remaining go to pool
    while (i < TOTAL)
    {
        poolIdx[poolCount] = i;
        poolCount = poolCount + 1;
        i = i + 1;
    }

    // --------------------------------------------------------
    // model weight (initialized to zero)
    // --------------------------------------------------------
    float* weight = new float[DIM];
    weight[0] = 0.0f;
    weight[1] = 0.0f;

    // --------------------------------------------------------
    // active learning loop
    // --------------------------------------------------------
    int iter = 0;
    while (iter < ACTIVE_ITER)
    {
        // 1) train on current labelled set
        train_on_set(weight, data, truth, labeledIdx, labCount, DIM);

        // 2) select most uncertain sample from pool
        int qPos = find_query(poolIdx, poolCount, weight, data, DIM);
        if (qPos == -1) break;            // pool empty

        // 3) move selected sample to labelled set
        int selected = poolIdx[qPos];
        labeledIdx[labCount] = selected;
        labCount = labCount + 1;

        // 4) compact pool (remove selected entry)
        int j = qPos;
        while (j < poolCount - 1)
        {
            poolIdx[j] = poolIdx[j + 1];
            j = j + 1;
        }
        poolCount = poolCount - 1;

        // 5) report iteration
        std::cout << "Iteration " << iter + 1
                  << ": queried point " << selected
                  << " (label " << truth[selected] << ")\n";

        iter = iter + 1;
    }

    // --------------------------------------------------------
    // final model report
    // --------------------------------------------------------
    std::cout << "\nFinal weight vector: ["
              << weight[0] << ", " << weight[1] << "]\n";

    std::cout << "Labelled indices after active learning: ";
    i = 0;
    while (i < labCount)
    {
        std::cout << labeledIdx[i];
        if (i < labCount - 1) std::cout << ", ";
        i = i + 1;
    }
    std::cout << "\n";

    // --------------------------------------------------------
    // clean‑up
    // --------------------------------------------------------
    delete[] data;
    delete[] truth;
    delete[] labeledIdx;
    delete[] poolIdx;
    delete[] weight;

    return 0;
}
