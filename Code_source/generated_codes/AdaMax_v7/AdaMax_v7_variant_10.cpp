#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

/* LLM input variant 10: large-safe-stress */

// ------------------- helper functions -------------------

// compute element‑wise max between two vectors (branch‑free style)
std::vector<float> vec_max(const std::vector<float>& a,
                           const std::vector<float>& b)
{
    std::vector<float> out(a.size());
    std::size_t i = 0;
    while (i < a.size())
    {
        float av = a[i];
        float bv = b[i];
        out[i] = std::max(av, bv);
        ++i;
    }
    return out;
}

// element‑wise absolute value
std::vector<float> vec_abs(const std::vector<float>& src)
{
    std::vector<float> out(src.size());
    std::size_t i = 0;
    while (i < src.size())
    {
        float v = src[i];
        out[i] = (v >= 0.0f) ? v : -v;
        ++i;
    }
    return out;
}

// element‑wise multiplication with scalar
std::vector<float> scalar_mul(const std::vector<float>& src, float k)
{
    std::vector<float> out(src.size());
    std::size_t i = 0;
    while (i < src.size())
    {
        out[i] = src[i] * k;
        ++i;
    }
    return out;
}

// element‑wise addition of two vectors
std::vector<float> vec_add(const std::vector<float>& a,
                           const std::vector<float>& b)
{
    std::vector<float> out(a.size());
    std::size_t i = 0;
    while (i < a.size())
    {
        out[i] = a[i] + b[i];
        ++i;
    }
    return out;
}

// element‑wise subtraction of two vectors
std::vector<float> vec_sub(const std::vector<float>& a,
                           const std::vector<float>& b)
{
    std::vector<float> out(a.size());
    std::size_t i = 0;
    while (i < a.size())
    {
        out[i] = a[i] - b[i];
        ++i;
    }
    return out;
}

// divide vector by scalar (adds epsilon for safety)
std::vector<float> vec_div_scalar(const std::vector<float>& num,
                                  float den, float eps)
{
    std::vector<float> out(num.size());
    float safe_den = den + eps;
    std::size_t i = 0;
    while (i < num.size())
    {
        out[i] = num[i] / safe_den;
        ++i;
    }
    return out;
}

// AdaMax single step (branch‑minimized)
void ada_max_step(std::vector<float>& params,
                  const std::vector<float>& grads,
                  std::vector<float>& m,
                  std::vector<float>& u,
                  float lr,
                  float b1,
                  float b2,
                  float eps,
                  int   step)
{
    float one_minus_b1 = 1.0f - b1;
    float one_minus_b2 = 1.0f - b2;

    std::vector<float> term1 = scalar_mul(m, b1);
    std::vector<float> term2 = scalar_mul(grads, one_minus_b1);
    std::vector<float> m_new = vec_add(term1, term2);

    std::vector<float> scaled_u = scalar_mul(u, b2);
    std::vector<float> abs_g = vec_abs(grads);
    std::vector<float> u_new = vec_max(scaled_u, abs_g);

    float b1_pow_t = std::pow(b1, static_cast<float>(step));
    float corr = 1.0f - b1_pow_t;

    std::vector<float> denom = vec_add(u_new, std::vector<float>(u_new.size(), eps));
    std::vector<float> step_vec(denom.size());
    std::size_t i = 0;
    while (i < denom.size())
    {
        step_vec[i] = m_new[i] / denom[i];
        ++i;
    }
    float scale = lr / corr;
    std::vector<float> update = scalar_mul(step_vec, scale);

    params = vec_sub(params, update);
    m = m_new;
    u = u_new;
}

// ------------------- main driver -------------------
int main()
{
    // version tag
    int version = 7;

    // hyper‑parameters (float only, no const)
    float learning_rate = 0.002f;
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float epsilon = 1e-8f;

    // large safe stress synthetic data
    const std::size_t N = 10000;               // size of vectors
    std::vector<float> weights(N);
    std::vector<float> grads(N);
    for (std::size_t i = 0; i < N; ++i)
    {
        // alternating sign and magnitude pattern, within safe float range
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        weights[i] = sign * static_cast<float>(i) * 0.001f;          // up to ~10.0
        grads[i]   = -sign * static_cast<float>(N - i) * 0.001f;    // opposite direction
    }

    // state vectors initialised to zero
    std::vector<float> m(weights.size(), 0.0f);
    std::vector<float> u(weights.size(), 0.0f);

    // run a few steps to expose edge cases
    int total_steps = 10;
    int step = 1;
    while (step <= total_steps)
    {
        ada_max_step(weights, grads, m, u,
                     learning_rate, beta1, beta2,
                     epsilon, step);
        ++step;
    }

    // print a subset of results to keep output manageable
    std::cout << "Version #" << version << " AdaMax result (first 20 params):\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(20, weights.size()); ++i)
    {
        std::cout << "param[" << i << "] = " << weights[i] << '\n';
    }
    return 0;
}
