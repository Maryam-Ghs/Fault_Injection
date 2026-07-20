/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cmath>
#include <vector>

void fillWeights(float* wgt, int sz)
{
    // initialize all to zero (sparse)
    for (int i = 0; i < sz; ++i)
        wgt[i] = 0.0f;

    // few non‑zero clustered values
    if (sz > 10)  wgt[10] = 0.5f;
    if (sz > 20)  wgt[20] = -0.3f;
    if (sz > 30)  wgt[30] = 0.8f;
    if (sz > 40)  wgt[40] = -0.1f;
    if (sz > 50)  wgt[50] = 0.6f;
    if (sz > 60)  wgt[60] = -0.2f;
    // leave the rest zero -> highly sparse
}

void fillGradients(float* gradArr, int sz)
{
    // initialize all to zero (sparse gradients)
    for (int i = 0; i < sz; ++i)
        gradArr[i] = 0.0f;

    // few non‑zero entries, clustered near the earlier weights
    if (sz > 10)  gradArr[10] = 0.02f;
    if (sz > 20)  gradArr[20] = -0.04f;
    if (sz > 30)  gradArr[30] = 0.01f;
    if (sz > 40)  gradArr[40] = -0.03f;
    if (sz > 50)  gradArr[50] = 0.015f;
    if (sz > 60)  gradArr[60] = -0.025f;
    // remaining entries stay zero
}

void adadeltaStep(float* wgt, float* gradArr,
                  float* accGrad, float* accUpd,
                  int sz, float r, float e)
{
    // generic loop handling arbitrary size, preserving original per‑element logic
    for (int i = 0; i < sz; ++i)
    {
        float g = gradArr[i];
        float gsq = g * g;
        float aG = r * accGrad[i] + (1.0f - r) * gsq;
        accGrad[i] = aG;
        float denom = std::sqrt(aG + e);
        float upd = std::sqrt(accUpd[i] + e) / denom * g;
        wgt[i] = wgt[i] - upd;
        float updsq = upd * upd;
        accUpd[i] = r * accUpd[i] + (1.0f - r) * updsq;
    }
}

void showArray(const char* tag, float* arr, int sz)
{
    std::cout << tag << ": ";
    for (int i = 0; i < sz; ++i)
    {
        std::cout << arr[i] << ' ';
    }
    std::cout << std::endl;
}

int main()
{
    const int dim = 1000;   // large dimension to emphasize sparsity

    // heap allocation
    float* wgt = new float[dim];
    float* gradArr = new float[dim];
    float* accGrad = new float[dim];
    float* accUpd = new float[dim];

    // initialise
    fillWeights(wgt, dim);
    fillGradients(gradArr, dim);
    for (int i = 0; i < dim; ++i)
    {
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }

    // AdaDelta hyper‑parameters
    const float r = 0.95f;   // rho
    const float e = 1e-6f;   // epsilon

    // run a few updates
    for (int step = 0; step < 5; ++step)
    {
        adadeltaStep(wgt, gradArr, accGrad, accUpd, dim, r, e);
    }

    // output final parameters (still sparse)
    const char nameW[] = "Final weights";
    showArray(nameW, wgt, dim);

    // clean up
    delete[] wgt;
    delete[] gradArr;
    delete[] accGrad;
    delete[] accUpd;

    return 0;
}
