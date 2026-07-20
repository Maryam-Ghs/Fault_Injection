#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

void fillWeights(float* wgt, int sz)
{
    // deterministic pseudo‑random‑looking initialization
    for (int i = 0; i < sz; ++i)
    {
        // values in range [-0.5, 0.49]
        wgt[i] = static_cast<float>((i * 37 % 100) - 50) / 100.0f;
    }
}

void fillGradients(float* gradArr, int sz)
{
    // deterministic pseudo‑random‑looking gradients (small magnitude)
    for (int i = 0; i < sz; ++i)
    {
        // values in range [-0.01, 0.0099]
        gradArr[i] = static_cast<float>((i * 53 % 100) - 50) / 5000.0f;
    }
}

void adadeltaStep(float* wgt, float* gradArr,
                  float* accGrad, float* accUpd,
                  int sz, float r, float e)
{
    // generic loop version handling any sz
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
    int dim = 12;   // medium‑sized deterministic test case

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
    float r = 0.95f;   // rho
    float e = 1e-6f;   // epsilon

    // run a few updates
    int step = 0;
    while (step < 5)
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
