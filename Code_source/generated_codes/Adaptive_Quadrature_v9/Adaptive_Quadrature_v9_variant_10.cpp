#include <iostream>

/* LLM input variant 10: large-safe-stress */

struct Segment{
    float start;
    float end;
    float eps;
};

float func(float x){
    // Example cubic function
    return x*x*x - 3.0f*x + 2.0f;
}

float simpsonRule(float a, float b, float fa, float fb, float fm){
    float h = b - a;                     // reorder: compute width first
    return h * (fa + 4.0f*fm + fb) / 6.0f;
}

float adaptRec(float a, float b, float fa, float fb, float fm,
               float whole, float eps, int depth){
    float mid = (a + b) * 0.5f;
    float leftMid  = (a + mid) * 0.5f;    // manual midpoint calculations
    float rightMid = (mid + b) * 0.5f;

    // manual loop unrolling for function evaluations
    float fl = func(leftMid);
    float fr = func(rightMid);

    float leftSim  = simpsonRule(a,   mid, fa, fm, fl);
    float rightSim = simpsonRule(mid, b,   fm, fb, fr);

    float diff = leftSim + rightSim - whole;
    if(depth <= 0 || (diff < 0.0f ? -diff : diff) < 15.0f*eps){
        return leftSim + rightSim + diff/15.0f;
    }
    // recurse on sub‑intervals, halving tolerance each time
    return adaptRec(a, mid, fa, fm, fl, leftSim, eps*0.5f, depth-1) +
           adaptRec(mid, b, fm, fb, fr, rightSim, eps*0.5f, depth-1);
}

float adaptiveIntegrate(float a, float b, float eps){
    float fa = func(a);
    float fb = func(b);
    float fm = func((a + b) * 0.5f);
    float whole = simpsonRule(a, b, fa, fb, fm);
    return adaptRec(a, b, fa, fb, fm, whole, eps, 20);
}

int main(){
    const int jobCount = 100; // increased number of intervals for stress testing
    // heap allocation for input intervals
    Segment* jobs = new Segment[jobCount];
    for(int i = 0; i < jobCount; ++i){
        jobs[i].start = i * 0.5f;          // start points spaced by 0.5
        jobs[i].end   = jobs[i].start + 2.0f; // each interval length = 2.0
        jobs[i].eps   = 1e-6f;            // tighter tolerance for more work
    }

    // heap allocation for results
    float* outcomes = new float[jobCount];

    // process all intervals
    for(int i = 0; i < jobCount; ++i){
        outcomes[i] = adaptiveIntegrate(jobs[i].start, jobs[i].end, jobs[i].eps);
    }

    std::cout << "Version 9 Adaptive Quadrature results (large-safe-stress variant):\n";
    for(int i = 0; i < jobCount; ++i){
        std::cout << "Interval " << i+1 << " [" << jobs[i].start
                  << ", " << jobs[i].end << "] = " << outcomes[i] << "\n";
    }

    delete[] jobs;
    delete[] outcomes;
    return 0;
}
