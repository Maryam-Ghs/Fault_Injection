#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

// fill a vector with random floats in [lo,hi) (kept for compatibility, not used)
void fill_vec(std::vector<float>& v, int n, float lo, float hi)
{
    for (int i = 0; i < n; ++i)
    {
        float r = (float)rand() / (float)RAND_MAX;
        v[i] = lo + r * (hi - lo);
    }
}

// one AdaMax update (manual 4‑way unrolling, temporaries split)
void adamax_update(const std::vector<float>& grad,
                   std::vector<float>& param,
                   std::vector<float>& m,
                   std::vector<float>& u,
                   int t,
                   float lr,
                   float b1,
                   float b2,
                   float eps)
{
    int sz = (int)grad.size();

    // bias‑correction term for m (using float powf)
    float b1t = powf(b1, (float)t);
    float corr = lr / (1.0f - b1t);

    int i = 0;
    // process four elements per iteration
    for (; i + 3 < sz; i += 4)
    {
        // ---- element i ----
        float g0 = grad[i];
        float m0 = b1 * m[i] + (1.0f - b1) * g0;
        float u0 = fmaxf(b2 * u[i], fabsf(g0));
        float step0 = corr * (m0 / (u0 + eps));
        float p0 = param[i] - step0;

        // ---- element i+1 ----
        float g1 = grad[i + 1];
        float m1 = b1 * m[i + 1] + (1.0f - b1) * g1;
        float u1 = fmaxf(b2 * u[i + 1], fabsf(g1));
        float step1 = corr * (m1 / (u1 + eps));
        float p1 = param[i + 1] - step1;

        // ---- element i+2 ----
        float g2 = grad[i + 2];
        float m2 = b1 * m[i + 2] + (1.0f - b1) * g2;
        float u2 = fmaxf(b2 * u[i + 2], fabsf(g2));
        float step2 = corr * (m2 / (u2 + eps));
        float p2 = param[i + 2] - step2;

        // ---- element i+3 ----
        float g3 = grad[i + 3];
        float m3 = b1 * m[i + 3] + (1.0f - b1) * g3;
        float u3 = fmaxf(b2 * u[i + 3], fabsf(g3));
        float step3 = corr * (m3 / (u3 + eps));
        float p3 = param[i + 3] - step3;

        // store back
        m[i]     = m0; u[i]     = u0; param[i]     = p0;
        m[i + 1] = m1; u[i + 1] = u1; param[i + 1] = p1;
        m[i + 2] = m2; u[i + 2] = u2; param[i + 2] = p2;
        m[i + 3] = m3; u[i + 3] = u3; param[i + 3] = p3;
    }

    // remaining elements (if any)
    for (; i < sz; ++i)
    {
        float g = grad[i];
        float mi = b1 * m[i] + (1.0f - b1) * g;
        float ui = fmaxf(b2 * u[i], fabsf(g));
        float step = corr * (mi / (ui + eps));
        float pi = param[i] - step;

        m[i] = mi;
        u[i] = ui;
        param[i] = pi;
    }
}

int main()
{
    // deterministic seed (not used, but kept for compatibility)
    srand((unsigned)time(nullptr));

    int dim = 8;                       // small size, multiple of 4
    std::vector<float> theta(dim, 1.0f);    // parameters set to 1
    std::vector<float> grads(dim, 0.0f);    // gradient placeholders
    std::vector<float> mom(dim, 0.0f); // first moment (m) initialized to 0
    std::vector<float> infnorm(dim, 0.0f); // infinity norm (u) initialized to 0

    // hyper‑parameters chosen to highlight identity behavior
    float learning = 1.0f;    // learning rate = 1
    float beta_one = 0.0f;    // no momentum effect
    float beta_two = 0.0f;    // no infinity‑norm accumulation
    float epsilon = 1e-8f;

    // run a few optimisation steps with deterministic gradient pattern
    for (int step = 1; step <= 5; ++step)
    {
        // pattern: alternating 0 and 1
        for (int i = 0; i < dim; ++i)
            grads[i] = (i % 2 == 0) ? 0.0f : 1.0f;

        adamax_update(grads, theta, mom, infnorm,
                      step, learning, beta_one, beta_two, epsilon);
    }

    // output final parameters
    for (int i = 0; i < dim; ++i)
        printf("%.6f ", theta[i]);
    printf("\n");

    return 0;
}
