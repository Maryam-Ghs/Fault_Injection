/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <cmath>

// maximum number of points we can store after refinement
#define MAX_PTS 2000

// number of points in the initial coarse grid (must be <= MAX_PTS)
#define INIT_PTS 50

// simple function we want to approximate (sine wave)
float target_func(float xv)
{
    // use sinf (float version) and return as float
    return sinf(xv);
}

// generate the initial coarse grid (0 .. 2*pi) with many points
void make_initial_grid(float *xp, float *yp, int &pt_cnt)
{
    pt_cnt = INIT_PTS;
    float step = 6.28318530f / static_cast<float>(INIT_PTS - 1); // 2*pi divided by (INIT_PTS-1)

    // fill x coordinates
    for (int i = 0; i < pt_cnt; ++i)
    {
        xp[i] = step * static_cast<float>(i);
    }

    // evaluate function values
    for (int i = 0; i < pt_cnt; ++i)
    {
        yp[i] = target_func(xp[i]);
    }
}

// one pass of adaptive refinement: insert mid‑points where linear error is too large
void one_refine_pass(float *xp, float *yp, int &pt_cnt, float tolerance)
{
    // temporary storage for the new set of points
    float nx[MAX_PTS];
    float ny[MAX_PTS];
    int new_cnt = 0;

    // always keep the first point
    nx[new_cnt] = xp[0];
    ny[new_cnt] = yp[0];
    ++new_cnt;

    // walk over each interval of the current mesh
    for (int seg = 0; seg < pt_cnt - 1; ++seg)
    {
        // left and right end of the interval
        float left_x  = xp[seg];
        float right_x = xp[seg + 1];
        float left_y  = yp[seg];
        float right_y = yp[seg + 1];

        // midpoint coordinate
        float mid_x = 0.5f * (left_x + right_x);
        // exact function value at midpoint
        float exact_mid_y = target_func(mid_x);
        // linear interpolation value at midpoint
        float lin_mid_y = 0.5f * (left_y + right_y);
        // absolute error of the linear approximation
        float err = fabsf(exact_mid_y - lin_mid_y);

        // if error exceeds tolerance, insert the midpoint
        if (err > tolerance && new_cnt < MAX_PTS - 1)
        {
            nx[new_cnt] = mid_x;
            ny[new_cnt] = exact_mid_y;
            ++new_cnt;
        }

        // always keep the right end of the interval (it will become left end of next)
        nx[new_cnt] = right_x;
        ny[new_cnt] = right_y;
        ++new_cnt;
    }

    // copy the refined mesh back to the original arrays
    for (int k = 0; k < new_cnt; ++k)
    {
        xp[k] = nx[k];
        yp[k] = ny[k];
    }
    pt_cnt = new_cnt;
}

// driver that performs a few refinement cycles
void adaptive_refine(float *xp, float *yp, int &pt_cnt)
{
    // tolerance for linear interpolation error (tighter to cause more insertions)
    float tol = 0.02f;

    // number of refinement passes
    int passes = 3;
    for (int p = 0; p < passes; ++p)
    {
        one_refine_pass(xp, yp, pt_cnt, tol);
    }
}

// print the final set of points
void dump_mesh(const float *xp, const float *yp, int pt_cnt)
{
    std::cout << "Adaptively refined mesh (point count = " << pt_cnt << "):\n";
    for (int i = 0; i < pt_cnt; ++i)
    {
        std::cout << "  x[" << i << "] = " << xp[i]
                  << " , y[" << i << "] = " << yp[i] << "\n";
    }
}

// -------------------------------------------------------------
int main()
{
    // storage for coordinates on the stack
    float xs[MAX_PTS];
    float ys[MAX_PTS];
    int npts = 0;

    // step 1: create a coarse initial grid
    make_initial_grid(xs, ys, npts);

    // step 2: perform adaptive refinement
    adaptive_refine(xs, ys, npts);

    // step 3: output the refined points
    dump_mesh(xs, ys, npts);

    return 0;
}
