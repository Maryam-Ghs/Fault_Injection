#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

// ------------------------------------------------------------
// generate an ordered, structured feature matrix (stack allocated)
// ------------------------------------------------------------
void gen_features(int nSamples, int nDim, float feats[][10])
{
    // Features increase linearly with sample and dimension indices,
    // creating a highly structured, nearly sorted dataset.
    for (int i = 0; i < nSamples; ++i)
    {
        for (int j = 0; j < nDim; ++j)
        {
            // Value ranges roughly from 0.0 to (nSamples+nDim)/1000
            feats[i][j] = static_cast<float>(i + j) * 0.001f;
        }
    }
}

// ------------------------------------------------------------
// generate deterministic true labels (float) based on structured features
// ------------------------------------------------------------
void gen_labels(int nSamples, const float feats[][10], int nDim, float trueLbl[])
{
    // true label = sum of features + a small deterministic pattern noise
    for (int i = 0; i < nSamples; ++i)
    {
        float tmpSum = 0.0f;
        for (int j = 0; j < nDim; ++j)
            tmpSum += feats[i][j];

        // deterministic noise cycles every 5 samples: -0.002, -0.001, 0.0, 0.001, 0.002
        int noisePattern = (i % 5) - 2;               // -2 .. 2
        float noise = static_cast<float>(noisePattern) * 0.001f;
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
    // seed (kept for compatibility, though not used in deterministic variant)
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

    // generate deterministic, ordered pool
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
