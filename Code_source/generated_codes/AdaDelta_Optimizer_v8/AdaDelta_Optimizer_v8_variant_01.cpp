/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cmath>

void fillWeights(float* wgt, int sz)
{
    // minimal initialization
    for (int i = 0; i < sz; ++i)
        wgt[i] = 0.0f;
}

void fillGradients(float* gradArr, int sz)
{
    // minimal gradients (zero)
    for (int i = 0; i < sz; ++i)
        gradArr[i] = 0.0f;
}

void adadeltaStep(float* wgt, float* gradArr,
                  float* accGrad, float* accUpd,
                  int sz, float r, float e)
{
    // manual unrolled loop for sz == 4
    // element 0
    float g0 = gradArr[0];
    float g0sq = g0 * g0;
    float aG0 = r * accGrad[0] + (1.0f - r) * g0sq;
    accGrad[0] = aG0;
    float denom0 = std::sqrt(aG0 + e);
    float upd0 = std::sqrt(accUpd[0] + e) / denom0 * g0;
    wgt[0] = wgt[0] - upd0;
    float upd0sq = upd0 * upd0;
    accUpd[0] = r * accUpd[0] + (1.0f - r) * upd0sq;

    // element 1
    float g1 = gradArr[1];
    float g1sq = g1 * g1;
    float aG1 = r * accGrad[1] + (1.0f - r) * g1sq;
    accGrad[1] = aG1;
    float denom1 = std::sqrt(aG1 + e);
    float upd1 = std::sqrt(accUpd[1] + e) / denom1 * g1;
    wgt[1] = wgt[1] - upd1;
    float upd1sq = upd1 * upd1;
    accUpd[1] = r * accUpd[1] + (1.0f - r) * upd1sq;

    // element 2
    float g2 = gradArr[2];
    float g2sq = g2 * g2;
    float aG2 = r * accGrad[2] + (1.0f - r) * g2sq;
    accGrad[2] = aG2;
    float denom2 = std::sqrt(aG2 + e);
    float upd2 = std::sqrt(accUpd[2] + e) / denom2 * g2;
    wgt[2] = wgt[2] - upd2;
    float upd2sq = upd2 * upd2;
    accUpd[2] = r * accUpd[2] + (1.0f - r) * upd2sq;

    // element 3
    float g3 = gradArr[3];
    float g3sq = g3 * g3;
    float aG3 = r * accGrad[3] + (1.0f - r) * g3sq;
    accGrad[3] = aG3;
    float denom3 = std::sqrt(aG3 + e);
    float upd3 = std::sqrt(accUpd[3] + e) / denom3 * g3;
    wgt[3] = wgt[3] - upd3;
    float upd3sq = upd3 * upd3;
    accUpd[3] = r * accUpd[3] + (1.0f - r) * upd3sq;
}

void showArray(char* tag, float* arr, int sz)
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
    int dim = 4;

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

    // AdaDelta hyper‑parameters (boundary values)
    float r = 0.0f;   // rho at lower bound
    float e = 0.0f;   // epsilon at lower bound

    // run a single update (minimal nontrivial step count)
    int step = 0;
    while (step < 1)
    {
        adadeltaStep(wgt, gradArr, accGrad, accUpd, dim, r, e);
        ++step;
    }

    // output final parameters
    char nameW[] = "Final weights";
    showArray(nameW, wgt, dim);

    // clean up
    delete[] wgt;
    delete[] gradArr;
    delete[] accGrad;
    delete[] accUpd;

    return 0;
}
