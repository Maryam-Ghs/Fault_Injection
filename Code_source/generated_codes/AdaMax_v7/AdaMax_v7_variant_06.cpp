#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

/* LLM input variant 6: ordered-structured */

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
        // std::max may contain a branch but keeps the source branch‑free
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
    float safe_den = den + eps;          // reorder: add epsilon before division
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
    // temporary scalars
    float one_minus_b1 = 1.0f - b1;
    float one_minus_b2 = 1.0f - b2;

    // ---- m_t = b1 * m_{t-1} + (1-b1) * g_t ----
    std::vector<float> term1 = scalar_mul(m, b1);
    std::vector<float> term2 = scalar_mul(grads, one_minus_b1);
    std::vector<float> m_new = vec_add(term1, term2);

    // ---- u_t = max(b2 * u_{t-1}, |g_t|) ----
    std::vector<float> scaled_u = scalar_mul(u, b2);
    std::vector<float> abs_g = vec_abs(grads);
    std::vector<float> u_new = vec_max(scaled_u, abs_g);

    // bias‑correction factor for m
    float b1_pow_t = std::pow(b1, static_cast<float>(step));
    float corr = 1.0f - b1_pow_t;

    // ---- update = (lr / corr) * (m_new / (u_new + eps)) ----
    std::vector<float> denom = vec_add(u_new, std::vector<float>(u_new.size(), eps));
    std::vector<float> step_vec(denom.size());
    std::size_t i = 0;
    while (i < denom.size())
    {
        step_vec[i] = m_new[i] / denom[i];
        ++i;
    }
    // scale by lr / corr
    float scale = lr / corr;
    std::vector<float> update = scalar_mul(step_vec, scale);

    // ---- params = params - update ----
    params = vec_sub(params, update);

    // write back m and u
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

    // ordered, symmetric synthetic data
    std::vector<float> weights = {-5.0f, -2.0f, 0.0f, 2.0f, 5.0f};
    std::vector<float> grads   = { 5.0f,  2.0f, 0.0f,-2.0f,-5.0f};

    // state vectors initialised to zero
    std::vector<float> m(weights.size(), 0.0f);
    std::vector<float> u(weights.size(), 0.0f);

    // run a few steps to expose edge cases
    int total_steps = 5;
    int step = 1;
    while (step <= total_steps)
    {
        ada_max_step(weights, grads, m, u,
                     learning_rate, beta1, beta2,
                     epsilon, step);
        ++step;
    }

    // print results
    std::cout << "Version #" << version << " AdaMax result:\n";
    std::size_t i = 0;
    while (i < weights.size())
    {
        std::cout << "param[" << i << "] = " << weights[i] << '\n';
        ++i;
    }
    return 0;
}
