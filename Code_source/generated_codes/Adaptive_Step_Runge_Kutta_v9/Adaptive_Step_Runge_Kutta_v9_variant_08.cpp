#include <iostream>
#include <cstdlib>
#include <cmath>

/* LLM input variant 8: sparse-skewed */
int main() {
    // version #9
    int Nsize = 10000;                     // large random array
    float *yInit = new float[Nsize];
    float *yRes  = new float[Nsize];

    // generate sparse and skewed initial conditions
    srand(12345);
    for (int i = 0; i < Nsize; ++i) {
        if (i >= 4950 && i < 5050) {
            // clustered region with normal range values
            yInit[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        } else if (i % 2000 == 0) {
            // few scattered tiny values (skewed towards zero)
            yInit[i] = (float)rand() / RAND_MAX * 0.01f - 0.005f;
        } else {
            // majority are zero
            yInit[i] = 0.0f;
        }
    }

    // integrate each problem independently
    for (int i = 0; i < Nsize; ++i) {
        float tcur = 0.0f;
        float ycur = yInit[i];
        float hcur = 0.1f;                 // initial step size

        while (tcur < 10.0f) {
            if (tcur + hcur > 10.0f) hcur = 10.0f - tcur;   // trim last step

            // -------- Dormand‑Prince coefficients (manual unrolling) ----------
            // f(t,y) = -y + sin(t)
            float k1 = hcur * ( -ycur + sinf(tcur) );

            float t2 = tcur + 0.2f * hcur;
            float y2 = ycur + 0.2f * k1;
            float k2 = hcur * ( -y2 + sinf(t2) );

            float t3 = tcur + 0.3f * hcur;
            float y3 = ycur + 0.075f * k1 + 0.225f * k2;
            float k3 = hcur * ( -y3 + sinf(t3) );

            float t4 = tcur + 0.8f * hcur;
            float y4 = ycur + 0.9777777778f * k1 - 3.733333333f * k2 + 3.555555556f * k3;
            float k4 = hcur * ( -y4 + sinf(t4) );

            float t5 = tcur + 8.0f/9.0f * hcur;
            float y5 = ycur + 2.952598689f * k1 - 11.59579321f * k2 + 9.822892851f * k3 + 0.290809327f * k4;
            float k5 = hcur * ( -y5 + sinf(t5) );

            float t6 = tcur + hcur;
            float y6 = ycur + 2.846275252f * k1 - 10.75713333f * k2 + 8.906422717f * k3
                       + 0.278409515f * k4 + 0.270598052f * k5;
            float k6 = hcur * ( -y6 + sinf(t6) );

            // 5th‑order solution (no extra stage needed for Dormand‑Prince)
            float y5th = ycur + 0.09114583333f * k1 - 0.07898950946f * k2
                         + 0.06744790104f * k3 + 0.05844444444f * k4
                         + 0.03750000000f * k5 + 0.02500000000f * k6;

            // 4th‑order solution (embedded)
            float y4th = ycur + 0.08991319444f * k1 - 0.07898950946f * k2
                         + 0.06962864070f * k3 + 0.05844444444f * k4
                         + 0.03600000000f * k5 + 0.02000000000f * k6;

            // error estimate
            float errEst = fabsf(y5th - y4th);

            // adapt step size
            if (errEst < 1e-4f) {
                tcur += hcur;
                ycur  = y5th;
                hcur *= 1.5f;                     // grow step
            } else {
                hcur *= 0.5f;                     // shrink step
                if (hcur < 1e-6f) hcur = 1e-6f;   // lower bound
            }
        }
        yRes[i] = ycur;
    }

    // print a sample of results
    for (int i = 0; i < 10; ++i) {
        std::cout << "y0[" << i << "]=" << yInit[i]
                  << "  ->  yFinal[" << i << "]=" << yRes[i] << std::endl;
    }

    delete[] yInit;
    delete[] yRes;
    return 0;
}
