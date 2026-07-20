#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 4: signed-extremes */

// ------------------------------------------------------------
// generate a random feature matrix (stack allocated)
// ------------------------------------------------------------
void gen_features(int nSamples, int nDim, float feats[][10])
{
    for (int i = 0; i < nSamples; ++i)
    {
        for (int j = 0; j < nDim; ++j)
        {
            int rnd = rand() % 10001 - 5000;          // -5000 .. 5000
            feats[i][j] = rnd * 0.001f;               // scale to [-5,5]
        }
    }
}

// ------------------------------------------------------------
// generate random true labels (float)
// ------------------------------------------------------------
void gen_labels(int nSamples, const float feats[][10], int nDim, float trueLbl[])
{
    // true label = sum of features + small noise
    for (int i = 0; i < nSamples; ++i)
    {
        float tmpSum = 0.0f;
        for (int j = 0; j < nDim; ++j)
            tmpSum += feats[i][j];

        int noise = rand() % 201 - 100;              // -100 .. 100
        trueLbl[i] = tmpSum + noise * 0.001f;
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
    // seed
    srand(12345);

    // problem size (stack allocated)
    int nSamples = 600;          // reduced for quicker demo
    int nDim     = 10;

    float X[800][10];
    float Ytrue[800];
    float weight[10];            // model parameters
    float uncertainty[800];
    bool  used[800];

    // initialise arrays
    for (int i = 0; i < nDim; ++i) weight[i] = 0.0f;
    for (int i = 0; i < nSamples; ++i) used[i] = false;

    // generate synthetic pool
    gen_features(nSamples, nDim, X);

    // impose explicit extreme rows: all zero, all max positive, all max negative
    for (int j = 0; j < nDim; ++j) X[0][j] = 0.0f;   // zero row
    for (int j = 0; j < nDim; ++j) X[1][j] = 5.0f;   // max positive
    for (int j = 0; j < nDim; ++j) X[2][j] = -5.0f;  // max negative

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
