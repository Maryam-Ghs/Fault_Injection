/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <cmath>

// -----------------------------------------------------------------
// Helper: 2‑D convolution (stride 1, no padding)
// -----------------------------------------------------------------
void convLayer(float* src, int srcC, int srcH, int srcW,
               float* ker, float* bias,
               int dstC, int kH, int kW,
               float* dst)
{
    int dstH = srcH - kH + 1;
    int dstW = srcW - kW + 1;

    int oc = 0;
    while (oc < dstC)
    {
        int oh = 0;
        while (oh < dstH)
        {
            int ow = 0;
            while (ow < dstW)
            {
                float acc = bias[oc];                     // start with bias
                int ic = 0;
                while (ic < srcC)
                {
                    int kh = 0;
                    while (kh < kH)
                    {
                        int kw = 0;
                        while (kw < kW)
                        {
                            int iH = oh + kh;
                            int iW = ow + kw;
                            float a = src[(ic * srcH + iH) * srcW + iW];
                            float w = ker[(((oc * srcC) + ic) * kH + kh) * kW + kw];
                            float prod = a * w;            // expanded multiplication
                            acc = acc + prod;               // expanded addition
                            ++kw;
                        }
                        ++kh;
                    }
                    ++ic;
                }
                dst[(oc * dstH + oh) * dstW + ow] = acc;
                ++ow;
            }
            ++oh;
        }
        ++oc;
    }
}

// -----------------------------------------------------------------
// Helper: ReLU (in‑place)
// -----------------------------------------------------------------
void reluLayer(float* data, int total)
{
    int i = 0;
    while (i < total)
    {
        // branch‑minimized using max function
        data[i] = std::fmax(0.0f, data[i]);
        ++i;
    }
}

// -----------------------------------------------------------------
// Helper: 2×2 max‑pool with stride 2 (in‑place)
// -----------------------------------------------------------------
void poolLayer(float* src, int srcC, int srcH, int srcW,
               float* dst)
{
    int dstH = srcH / 2;
    int dstW = srcW / 2;

    int c = 0;
    while (c < srcC)
    {
        int ph = 0;
        while (ph < dstH)
        {
            int pw = 0;
            while (pw < dstW)
            {
                float m0 = src[(c * srcH + ph * 2) * srcW + pw * 2];
                float m1 = src[(c * srcH + ph * 2) * srcW + pw * 2 + 1];
                float m2 = src[(c * srcH + ph * 2 + 1) * srcW + pw * 2];
                float m3 = src[(c * srcH + ph * 2 + 1) * srcW + pw * 2 + 1];
                // expanded max
                float t0 = (m0 > m1) ? m0 : m1;
                float t1 = (m2 > m3) ? m2 : m3;
                float t2 = (t0 > t1) ? t0 : t1;
                dst[(c * dstH + ph) * dstW + pw] = t2;
                ++pw;
            }
            ++ph;
        }
        ++c;
    }
}

// -----------------------------------------------------------------
// Helper: Fully connected (in‑place output)
// -----------------------------------------------------------------
void fcLayer(float* inp, int inSize,
             float* w, float* b,
             int outSize,
             float* out)
{
    int o = 0;
    while (o < outSize)
    {
        float sum = b[o];
        int i = 0;
        while (i < inSize)
        {
            float prod = inp[i] * w[o * inSize + i];
            sum = sum + prod;
            ++i;
        }
        out[o] = sum;
        ++o;
    }
}

// -----------------------------------------------------------------
// Helper: Softmax (in‑place)
// -----------------------------------------------------------------
void softmaxLayer(float* vec, int sz)
{
    int i = 0;
    float maxv = vec[0];
    while (i < sz)
    {
        maxv = (vec[i] > maxv) ? vec[i] : maxv;
        ++i;
    }

    i = 0;
    float sum = 0.0f;
    while (i < sz)
    {
        vec[i] = std::exp(vec[i] - maxv);
        sum = sum + vec[i];
        ++i;
    }

    i = 0;
    while (i < sz)
    {
        vec[i] = vec[i] / sum;
        ++i;
    }
}

// -----------------------------------------------------------------
// Main – tiny AlexNet‑style network
// -----------------------------------------------------------------
int main()
{
    // -------------------------------------------------------------
    // 1. Input: 3 × 8 × 8 (ordered ascending values)
    // -------------------------------------------------------------
    int inC = 3, inH = 8, inW = 8;
    float* img = new float[inC * inH * inW];
    int idx = 0;
    while (idx < inC * inH * inW)
    {
        img[idx] = static_cast<float>(idx + 1);   // strictly increasing values
        ++idx;
    }

    // -------------------------------------------------------------
    // 2. Conv1: 2 filters, 3×3 kernel (ordered increasing weights)
    // -------------------------------------------------------------
    int c1Out = 2, k1 = 3;
    float* w1 = new float[c1Out * inC * k1 * k1];
    float* b1 = new float[c1Out];
    idx = 0;
    while (idx < c1Out * inC * k1 * k1)
    {
        w1[idx] = 0.01f * static_cast<float>(idx + 1); // increasing weight per element
        ++idx;
    }
    idx = 0;
    while (idx < c1Out)
    {
        b1[idx] = 0.0f;
        ++idx;
    }
    int c1H = inH - k1 + 1;
    int c1W = inW - k1 + 1;
    float* out1 = new float[c1Out * c1H * c1W];
    convLayer(img, inC, inH, inW, w1, b1, c1Out, k1, k1, out1);
    reluLayer(out1, c1Out * c1H * c1W);

    // -------------------------------------------------------------
    // 3. Pool1: 2×2, stride 2
    // -------------------------------------------------------------
    int p1H = c1H / 2;
    int p1W = c1W / 2;
    float* outP1 = new float[c1Out * p1H * p1W];
    poolLayer(out1, c1Out, c1H, c1W, outP1);

    // -------------------------------------------------------------
    // 4. Conv2: 2 filters, 3×3 kernel (ordered increasing weights)
    // -------------------------------------------------------------
    int c2Out = 2, k2 = 3;
    float* w2 = new float[c2Out * c1Out * k2 * k2];
    float* b2 = new float[c2Out];
    idx = 0;
    while (idx < c2Out * c1Out * k2 * k2)
    {
        w2[idx] = 0.005f * static_cast<float>(idx + 1);
        ++idx;
    }
    idx = 0;
    while (idx < c2Out)
    {
        b2[idx] = 0.0f;
        ++idx;
    }
    int c2H = p1H - k2 + 1;
    int c2W = p1W - k2 + 1;
    float* out2 = new float[c2Out * c2H * c2W];
    convLayer(outP1, c1Out, p1H, p1W, w2, b2, c2Out, k2, k2, out2);
    reluLayer(out2, c2Out * c2H * c2W);

    // -------------------------------------------------------------
    // 5. Flatten
    // -------------------------------------------------------------
    int flatSize = c2Out * c2H * c2W;
    float* flat = new float[flatSize];
    idx = 0;
    while (idx < flatSize)
    {
        flat[idx] = out2[idx];
        ++idx;
    }

    // -------------------------------------------------------------
    // 6. FC1: 4 neurons (ordered increasing weights)
    // -------------------------------------------------------------
    int fc1Out = 4;
    float* wfc1 = new float[fc1Out * flatSize];
    float* bfc1 = new float[fc1Out];
    idx = 0;
    while (idx < fc1Out * flatSize)
    {
        wfc1[idx] = 0.002f * static_cast<float>(idx + 1);
        ++idx;
    }
    idx = 0;
    while (idx < fc1Out)
    {
        bfc1[idx] = 0.0f;
        ++idx;
    }
    float* outFc1 = new float[fc1Out];
    fcLayer(flat, flatSize, wfc1, bfc1, fc1Out, outFc1);
    reluLayer(outFc1, fc1Out);

    // -------------------------------------------------------------
    // 7. FC2: 2 output classes (ordered increasing weights)
    // -------------------------------------------------------------
    int fc2Out = 2;
    float* wfc2 = new float[fc2Out * fc1Out];
    float* bfc2 = new float[fc2Out];
    idx = 0;
    while (idx < fc2Out * fc1Out)
    {
        wfc2[idx] = 0.003f * static_cast<float>(idx + 1);
        ++idx;
    }
    idx = 0;
    while (idx < fc2Out)
    {
        bfc2[idx] = 0.0f;
        ++idx;
    }
    float* outFc2 = new float[fc2Out];
    fcLayer(outFc1, fc1Out, wfc2, bfc2, fc2Out, outFc2);
    softmaxLayer(outFc2, fc2Out);

    // -------------------------------------------------------------
    // 8. Print result
    // -------------------------------------------------------------
    std::cout << "Probabilities:" << std::endl;
    idx = 0;
    while (idx < fc2Out)
    {
        std::cout << "Class " << idx << ": " << outFc2[idx] << std::endl;
        ++idx;
    }

    // -------------------------------------------------------------
    // 9. Clean‑up
    // -------------------------------------------------------------
    delete[] img;
    delete[] w1; delete[] b1; delete[] out1;
    delete[] outP1;
    delete[] w2; delete[] b2; delete[] out2;
    delete[] flat;
    delete[] wfc1; delete[] bfc1; delete[] outFc1;
    delete[] wfc2; delete[] bfc2; delete[] outFc2;

    return 0;
}
