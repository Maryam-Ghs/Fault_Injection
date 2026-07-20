#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

// simple test function: f(x) = sin(pi * x)
float fun(float x)
{
    // pi as a float literal
    return sinf(3.1415926f * x);
}

// set up the initial interval with a skewed, clustered distribution
void start(std::vector<float>& pts, std::vector<float>& vals)
{
    // cluster many points near 0 and a few far points
    float init_points[] = {0.0f, 0.0001f, 0.0002f, 0.001f, 0.01f, 0.5f, 1.0f};
    int n = sizeof(init_points) / sizeof(init_points[0]);
    for (int i = 0; i < n; ++i)
    {
        pts.push_back(init_points[i]);
        vals.push_back(fun(init_points[i]));
    }
}

// perform one refinement pass; return true if a new point was added
bool one_pass(std::vector<float>& pts, std::vector<float>& vals, float tol)
{
    int i = 0;
    // loop until a refinement is performed or end is reached
    while (i < (int)pts.size() - 1)
    {
        // midpoint of the current segment
        float mid = (pts[i] + pts[i + 1]) * 0.5f;
        // actual function value at the midpoint
        float fmid = fun(mid);
        // linear interpolation at the midpoint
        float lin = (vals[i] + vals[i + 1]) * 0.5f;
        // absolute error
        float err = fabsf(fmid - lin);

        // if error exceeds tolerance, insert the new point
        if (err > tol)
        {
            pts.insert(pts.begin() + i + 1, mid);
            vals.insert(vals.begin() + i + 1, fmid);
            return true;            // a refinement was made
        }
        ++i;                         // advance to next segment
    }
    return false;                    // no refinement needed
}

// adaptively refine until the whole representation meets the tolerance
void adapt(std::vector<float>& pts, std::vector<float>& vals, float tol)
{
    // repeat passes until no further points are added
    bool changed = true;
    while (changed)
    {
        changed = one_pass(pts, vals, tol);
    }
}

// display the final points and their function values
void show(std::vector<float>& pts, std::vector<float>& vals)
{
    int cnt = (int)pts.size();
    std::cout << "Number of points: " << cnt << "\n";
    std::cout << "   x        f(x)\n";
    for (int k = 0; k < cnt; ++k)
    {
        std::cout << pts[k] << "  " << vals[k] << "\n";
    }
}

int main()
{
    // deterministic test vector: start with a skewed, clustered set of points
    std::vector<float> xs;
    std::vector<float> ys;

    start(xs, ys);

    // tolerance for adaptive refinement
    float limit = 0.01f;

    // run the adaptive algorithm
    adapt(xs, ys, limit);

    // output the refined grid
    show(xs, ys);

    return 0;
}
