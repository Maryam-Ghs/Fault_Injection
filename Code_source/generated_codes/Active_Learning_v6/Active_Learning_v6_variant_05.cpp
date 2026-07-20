#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 5: duplicate-heavy */

// ------------------------------------------------------------
// generate a deterministic feature matrix with many duplicate rows (stack allocated)
// ------------------------------------------------------------
void gen_features(int nSamples, int nDim, float feats[][10])
{
    // define a small set of base feature vectors to be reused
    const float base[5][10] = {
        { 0.10f, -0.20f,  0.30f, -0.40f,  0.50f, -0.60f,  0.70f, -0.80f,  0.90f, -0.10f},
        { 0.11f, -0.21f,  0.31f, -0.41f,  0.51f, -0.61f,  0.71f, -0.81f,  0.91f, -0.11f},
        { 0.12f, -0.22f,  0.32f, -0.42f,  0.52f, -0.62f,  0.72f, -0.82f,  0.92f, -0.12f},
        { 0.13f, -0.23f,  0.33f, -0.43f,  0.53f, -0.63f,  0.73f, -0.83f,  0.93f, -0.13f},
        { 0.14f, -0.24f,  0.34f, -0.44f,  0.54f, -0.64f,  0.74f, -0.84f,  0.94f, -0.14f}
    };
    for (int i = 0; i < nSamples; ++i)
    {
        const float* src = base[i % 5];
        for (int j = 0; j < nDim; ++j)
        {
            feats[i][j] = src[j];
        }
    }
}

// ------------------------------------------------------------
// generate deterministic true labels (float) with repeated patterns
// ------------------------------------------------------------
void gen_labels(int nSamples, const float feats[][10], int nDim, float trueLbl[])
{
    // small set of noise values to repeat
    const float noiseVals[5] = { 0.001f, -0.002f, 0.003f, -0.004f, 0.005f };
    for (int i = 0; i < nSamples; ++i)
    {
        float tmpSum = 0.0f;
        for (int j = 0; j < nDim; ++j)
            tmpSum += feats[i][j];
        float noise = noiseVals[i % 5];
        trueLbl[i] = tmpSum + noise;
    }
}

// ------------------------------------------------------------
// simple linear prediction: dot(product) of weight and feature
// ------------------------------------------------------------
float predict(const float w[], const float x[], int nDim)
{
    float acc = 0.0f;
    for (int i = 0; i < nDim; ++i)
        acc += w[i] * x[i];
    return acc;
}

// ------------------------------------------------------------
// compute uncertainty = |prediction - true|
// ------------------------------------------------------------
void calc_uncertainty(int nSamples, int nDim,
                      const float feats[][10],
                      const float w[],
                      const float trueLbl[],
                      float unc[])
{
    for (int i = 0; i < nSamples; ++i)
    {
        float pred = predict(w, feats[i], nDim);
        float diff = pred - trueLbl[i];
        if (diff < 0.0f) diff = -diff;
        unc[i] = diff;
    }
}

// ------------------------------------------------------------
// pick the index of the largest uncertainty among not yet chosen
// ------------------------------------------------------------
int pick_max_unc(int nSamples, const float unc[], const bool taken[])
{
    int bestIdx = -1;
    float bestVal = -1.0f;
    for (int i = 0; i < nSamples; ++i)
    {
        if (taken[i]) continue;
        if (unc[i] > bestVal)
        {
            bestVal = unc[i];
            bestIdx = i;
        }
    }
    return bestIdx;
}

// ------------------------------------------------------------
// simple SGD weight update (learning rate = 0.01)
// ------------------------------------------------------------
void sgd_update(float w[], const float x[], float target, int nDim)
{
    float pred = predict(w, x, nDim);
    float err  = target - pred;          // error term

    // split learning rate multiplication into temp vars
    float lr   = 0.01f;
    float step = lr * err;

    for (int i = 0; i < nDim; ++i)
        w[i] += step * x[i];
}

// ------------------------------------------------------------
int main()
{
    // seed (kept for reproducibility, though not used for randomness now)
    srand(12345);

    // problem size (stack allocated)
    int nSamples = 800;          // large enough for demo
    int nDim     = 10;

    float X[800][10];
    float Ytrue[800];
    float weight[10];            // model parameters
    float uncertainty[800];
    bool  used[800];

    // initialise arrays
    for (int i = 0; i < nDim; ++i) weight[i] = 0.0f;
    for (int i = 0; i < nSamples; ++i) used[i] = false;

    // generate synthetic pool with duplicate-heavy data
    gen_features(nSamples, nDim, X);
    gen_labels(nSamples, X, nDim, Ytrue);

    // active learning loop
    int budget = 30;                     // how many samples we may label
    int iter   = 0;
    while (iter < budget)
    {
        // recompute uncertainties based on current model
        calc_uncertainty(nSamples, nDim, X, weight, Ytrue, uncertainty);

        // select most uncertain example not yet taken
        int sel = pick_max_unc(nSamples, uncertainty, used);
        if (sel == -1) break;            // no more candidates

        // mark as used
        used[sel] = true;

        // simulate obtaining the label (here we already have Ytrue)
        float label = Ytrue[sel];

        // update model with the newly labeled example
        sgd_update(weight, X[sel], label, nDim);

        ++iter;
    }

    // print final weight vector
    printf("Final model weights after %d updates:\n", iter);
    for (int i = 0; i < nDim; ++i)
        printf(" w[%d] = %.5f\n", i, weight[i]);

    // report how many samples were actually selected
    int takenCnt = 0;
    for (int i = 0; i < nSamples; ++i)
        if (used[i]) ++takenCnt;
    printf("Total samples labeled: %d out of %d\n", takenCnt, nSamples);

    return 0;
}
