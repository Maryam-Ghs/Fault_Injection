#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

// version #7

// evaluate the target function (sine)
float evalFunc(float x) {
    return sinf(x);
}

// adaptive refinement of a 1‑D mesh
void refineMesh(float *& meshX, float *& meshY, int &cnt, float eps) {
    while (true) {
        // first pass: detect intervals that need refinement
        int extra = 0;
        for (int i = 0; i < cnt - 1; ++i) {
            float mid = 0.5f * (meshX[i] + meshX[i + 1]);
            float fmid = evalFunc(mid);
            float linY = meshY[i] + (meshY[i + 1] - meshY[i]) *
                         ((mid - meshX[i]) / (meshX[i + 1] - meshX[i]));
            if (fabsf(fmid - linY) > eps) ++extra;
        }
        // no interval violates the tolerance → stop
        if (extra == 0) break;

        // allocate enlarged arrays
        int newCnt = cnt + extra;
        float *newX = new float[newCnt];
        float *newY = new float[newCnt];

        // second pass: copy old points and insert new ones where needed
        int pos = 0;
        for (int i = 0; i < cnt - 1; ++i) {
            newX[pos] = meshX[i];
            newY[pos] = meshY[i];
            ++pos;

            float mid = 0.5f * (meshX[i] + meshX[i + 1]);
            float fmid = evalFunc(mid);
            float linY = meshY[i] + (meshY[i + 1] - meshY[i]) *
                         ((mid - meshX[i]) / (meshX[i + 1] - meshX[i]));
            if (fabsf(fmid - linY) > eps) {
                newX[pos] = mid;
                newY[pos] = fmid;
                ++pos;
            }
        }
        // copy the final original point
        newX[pos] = meshX[cnt - 1];
        newY[pos] = meshY[cnt - 1];

        // replace old storage
        delete[] meshX;
        delete[] meshY;
        meshX = newX;
        meshY = newY;
        cnt   = newCnt;
    }
}

int main() {
    // initial structured mesh: equally spaced points from 0 to 2π
    const int size = 5;
    float *xs = new float[size];
    float *ys = new float[size];

    const float pi = 3.14159265358979323846f;
    const float step = (2.0f * pi) / (size - 1);
    for (int i = 0; i < size; ++i) {
        xs[i] = i * step;
    }

    for (int i = 0; i < size; ++i) {
        ys[i] = evalFunc(xs[i]);
    }

    float tolerance = 0.05f;   // allowed linear interpolation error

    // perform adaptive refinement
    int cnt = size;
    refineMesh(xs, ys, cnt, tolerance);

    // output the refined mesh
    for (int i = 0; i < cnt; ++i) {
        std::cout << "x[" << i << "] = " << xs[i]
                  << "   f(x) = " << ys[i] << '\n';
    }

    delete[] xs;
    delete[] ys;
    return 0;
}
