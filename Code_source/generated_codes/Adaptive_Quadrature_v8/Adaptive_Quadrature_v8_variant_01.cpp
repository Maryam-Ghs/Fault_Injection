/* LLM input variant 1: minimal-boundary */
// Adaptive Quadrature implementation – version #8
// Uses only float and int, no const, no double, no unsigned, no long.

#include <iostream>
#include <vector>
#include <utility>

// Simple test function: f(x) = x*x + 2*x + 1
float test_func(float x) {
    // expanded multi‑step computation
    float x2 = x * x;
    float two_x = 2.0f * x;
    float result = x2 + two_x + 1.0f;
    return result;
}

// Simpson rule on a single interval [p, q]
float simpson_rule(float (*fun)(float), float p, float q) {
    // expanded steps
    float half = (p + q) * 0.5f;
    float fp = fun(p);
    float fm = fun(half);
    float fq = fun(q);
    float seg_len = q - p;
    float coeff = seg_len / 6.0f;
    float sim = coeff * (fp + 4.0f * fm + fq);
    return sim;
}

// Adaptive integration helper – recursive
float adaptive_helper(float (*fun)(float), float left, float right,
                      float tol, float whole, int depth) {
    // stop condition to avoid infinite recursion
    if (depth > 30) {
        return whole;
    }

    // compute mid‑point and sub‑simpson values
    float mid = (left + right) * 0.5f;
    float left_sim  = simpson_rule(fun, left,  mid);
    float right_sim = simpson_rule(fun, mid,   right);
    float combined  = left_sim + right_sim;

    // error estimate (absolute)
    float err = combined - whole;
    if (err < 0.0f) err = -err;

    // if error acceptable, return corrected value
    if (err < 15.0f * tol) {
        // Richardson extrapolation
        return combined + (combined - whole) / 15.0f;
    }

    // otherwise recurse on sub‑intervals
    float left_res  = adaptive_helper(fun, left,  mid, tol * 0.5f,
                                      left_sim,  depth + 1);
    float right_res = adaptive_helper(fun, mid, right, tol * 0.5f,
                                      right_sim, depth + 1);
    return left_res + right_res;
}

// Public adaptive quadrature interface
float adaptive_integrate(float (*fun)(float), float a, float b, float eps) {
    float init = simpson_rule(fun, a, b);
    return adaptive_helper(fun, a, b, eps, init, 0);
}

// Class encapsulating the whole process
class AdaptiveQuad {
public:
    // vector of jobs: each job is (a, b, tolerance)
    std::vector< std::pair< std::pair<float,float>, float > > jobs;

    // fill jobs with minimal predefined data
    void prepare_jobs() {
        // using a while loop instead of for
        int idx = 0;
        while (idx < 1) {
            // integrate test_func on a minimal non‑zero interval [0, 0.001] with eps = 1e-6
            jobs.push_back(std::make_pair(std::make_pair(0.0f, 0.001f), 0.000001f));
            idx = idx + 1;
        }
    }

    // run all jobs and collect results
    std::vector<float> run_all() {
        std::vector<float> outcomes;
        int i = 0;
        // using a for‑loop with manual increment
        for (i = 0; i < (int)jobs.size(); ) {
            float left = jobs[i].first.first;
            float right = jobs[i].first.second;
            float tol = jobs[i].second;
            // choose function based on interval (demo purpose)
            float (*fun_ptr)(float) = (i == 0) ? [](float x){ return x * x; } : test_func;
            float res = adaptive_integrate(fun_ptr, left, right, tol);
            outcomes.push_back(res);
            i = i + 1;   // explicit increment
        }
        return outcomes;
    }

    // display results
    void show(const std::vector<float>& data) {
        int k = 0;
        // using while loop for printing
        while (k < (int)data.size()) {
            std::cout << "Result " << (k + 1) << " : " << data[k] << "\n";
            k = k + 1;
        }
    }
};

int main() {
    AdaptiveQuad engine;
    engine.prepare_jobs();                // generate internal input
    std::vector<float> answers = engine.run_all();
    engine.show(answers);                 // print the outcomes
    return 0;
}
