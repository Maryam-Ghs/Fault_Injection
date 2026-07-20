#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 4: signed-extremes */

#define MAX_SAMPLES 8
#define MAX_ITER    3

/*  AdaBoost Regressor with decision‑stump weak learners.
    All data lives on the stack, only float/int are used,
    loops are manually unrolled and intermediate results are
    stored in temporary variables.                                           */

class AdaBoostReg {
    /* one‑dimensional decision stump */
    struct Stump {
        float thr;      // split threshold
        float left;     // prediction for x <= thr
        float right;    // prediction for x >  thr
        float weight;   // learner weight (learning rate)
    };

    Stump learners[MAX_ITER];
    int    n_iter;          // actual number of learners
    int    n_samples;       // training size

public:
    AdaBoostReg() : n_iter(0), n_samples(0) {}

    /* fit on fixed‑size data (features are 1‑D) */
    void fit(const float X[MAX_SAMPLES], const float Y[MAX_SAMPLES], int N) {
        n_samples = N;
        /* initial prediction = 0, residual = Y */
        float pred[MAX_SAMPLES];
        float resid[MAX_SAMPLES];

        // initialise predictions and residuals
        pred[0] = 0.0f; pred[1] = 0.0f; pred[2] = 0.0f; pred[3] = 0.0f;
        pred[4] = 0.0f; pred[5] = 0.0f; pred[6] = 0.0f; pred[7] = 0.0f;

        resid[0] = Y[0]; resid[1] = Y[1]; resid[2] = Y[2]; resid[3] = Y[3];
        resid[4] = Y[4]; resid[5] = Y[5]; resid[6] = Y[6]; resid[7] = Y[7];

        // uniform sample weights
        float wgt[MAX_SAMPLES];
        float uniform = 1.0f / (float)N;
        wgt[0] = uniform; wgt[1] = uniform; wgt[2] = uniform; wgt[3] = uniform;
        wgt[4] = uniform; wgt[5] = uniform; wgt[6] = uniform; wgt[7] = uniform;

        for (int it = 0; it < MAX_ITER; ++it) {
            /* ---- find best stump on current residuals ---- */
            float best_err = 1e30f;
            float best_thr = 0.0f, best_l = 0.0f, best_r = 0.0f;

            // try every training point as threshold
            // (manual unrolling of the inner loop over samples)
            for (int t = 0; t < N; ++t) {
                float thr = X[t];

                // accumulate weighted sums for left/right partitions
                float sumL = 0.0f, sumR = 0.0f;
                float wL   = 0.0f, wR   = 0.0f;

                // sample 0
                if (X[0] <= thr) { sumL += wgt[0] * resid[0]; wL += wgt[0]; }
                else               { sumR += wgt[0] * resid[0]; wR += wgt[0]; }
                // sample 1
                if (X[1] <= thr) { sumL += wgt[1] * resid[1]; wL += wgt[1]; }
                else               { sumR += wgt[1] * resid[1]; wR += wgt[1]; }
                // sample 2
                if (X[2] <= thr) { sumL += wgt[2] * resid[2]; wL += wgt[2]; }
                else               { sumR += wgt[2] * resid[2]; wR += wgt[2]; }
                // sample 3
                if (X[3] <= thr) { sumL += wgt[3] * resid[3]; wL += wgt[3]; }
                else               { sumR += wgt[3] * resid[3]; wR += wgt[3]; }
                // sample 4
                if (X[4] <= thr) { sumL += wgt[4] * resid[4]; wL += wgt[4]; }
                else               { sumR += wgt[4] * resid[4]; wR += wgt[4]; }
                // sample 5
                if (X[5] <= thr) { sumL += wgt[5] * resid[5]; wL += wgt[5]; }
                else               { sumR += wgt[5] * resid[5]; wR += wgt[5]; }
                // sample 6
                if (X[6] <= thr) { sumL += wgt[6] * resid[6]; wL += wgt[6]; }
                else               { sumR += wgt[6] * resid[6]; wR += wgt[6]; }
                // sample 7
                if (X[7] <= thr) { sumL += wgt[7] * resid[7]; wL += wgt[7]; }
                else               { sumR += wgt[7] * resid[7]; wR += wgt[7]; }

                // compute predictions for each side (weighted averages)
                float predL = (wL > 0.0f) ? (sumL / wL) : 0.0f;
                float predR = (wR > 0.0f) ? (sumR / wR) : 0.0f;

                // weighted squared error of this stump
                float err = 0.0f;
                // sample 0 contribution
                float diff0 = (X[0] <= thr) ? (resid[0] - predL) : (resid[0] - predR);
                err += wgt[0] * diff0 * diff0;
                // sample 1
                float diff1 = (X[1] <= thr) ? (resid[1] - predL) : (resid[1] - predR);
                err += wgt[1] * diff1 * diff1;
                // sample 2
                float diff2 = (X[2] <= thr) ? (resid[2] - predL) : (resid[2] - predR);
                err += wgt[2] * diff2 * diff2;
                // sample 3
                float diff3 = (X[3] <= thr) ? (resid[3] - predL) : (resid[3] - predR);
                err += wgt[3] * diff3 * diff3;
                // sample 4
                float diff4 = (X[4] <= thr) ? (resid[4] - predL) : (resid[4] - predR);
                err += wgt[4] * diff4 * diff4;
                // sample 5
                float diff5 = (X[5] <= thr) ? (resid[5] - predL) : (resid[5] - predR);
                err += wgt[5] * diff5 * diff5;
                // sample 6
                float diff6 = (X[6] <= thr) ? (resid[6] - predL) : (resid[6] - predR);
                err += wgt[6] * diff6 * diff6;
                // sample 7
                float diff7 = (X[7] <= thr) ? (resid[7] - predL) : (resid[7] - predR);
                err += wgt[7] * diff7 * diff7;

                // keep best
                if (err < best_err) {
                    best_err = err;
                    best_thr = thr;
                    best_l   = predL;
                    best_r   = predR;
                }
            }

            // compute learner weight (simple inverse error scaling)
            float eps = 1e-6f;
            float alpha = 0.5f * logf((1.0f - best_err + eps) / (best_err + eps));

            // store learner
            learners[it].thr    = best_thr;
            learners[it].left   = best_l;
            learners[it].right  = best_r;
            learners[it].weight = alpha;

            // update ensemble prediction and residuals
            // (manual unrolling for each sample)
            // sample 0
            float pred0 = (X[0] <= best_thr) ? best_l : best_r;
            pred[0] += alpha * pred0;
            resid[0] = Y[0] - pred[0];
            // sample 1
            float pred1 = (X[1] <= best_thr) ? best_l : best_r;
            pred[1] += alpha * pred1;
            resid[1] = Y[1] - pred[1];
            // sample 2
            float pred2 = (X[2] <= best_thr) ? best_l : best_r;
            pred[2] += alpha * pred2;
            resid[2] = Y[2] - pred[2];
            // sample 3
            float pred3 = (X[3] <= best_thr) ? best_l : best_r;
            pred[3] += alpha * pred3;
            resid[3] = Y[3] - pred[3];
            // sample 4
            float pred4 = (X[4] <= best_thr) ? best_l : best_r;
            pred[4] += alpha * pred4;
            resid[4] = Y[4] - pred[4];
            // sample 5
            float pred5 = (X[5] <= best_thr) ? best_l : best_r;
            pred[5] += alpha * pred5;
            resid[5] = Y[5] - pred[5];
            // sample 6
            float pred6 = (X[6] <= best_thr) ? best_l : best_r;
            pred[6] += alpha * pred6;
            resid[6] = Y[6] - pred[6];
            // sample 7
            float pred7 = (X[7] <= best_thr) ? best_l : best_r;
            pred[7] += alpha * pred7;
            resid[7] = Y[7] - pred[7];

            // re‑weight samples (AdaBoost style)
            // w_i <- w_i * exp(alpha * |residual_i|)
            // then renormalise
            float sumw = 0.0f;
            // sample 0
            wgt[0] = wgt[0] * expf(alpha * fabsf(resid[0])); sumw += wgt[0];
            // sample 1
            wgt[1] = wgt[1] * expf(alpha * fabsf(resid[1])); sumw += wgt[1];
            // sample 2
            wgt[2] = wgt[2] * expf(alpha * fabsf(resid[2])); sumw += wgt[2];
            // sample 3
            wgt[3] = wgt[3] * expf(alpha * fabsf(resid[3])); sumw += wgt[3];
            // sample 4
            wgt[4] = wgt[4] * expf(alpha * fabsf(resid[4])); sumw += wgt[4];
            // sample 5
            wgt[5] = wgt[5] * expf(alpha * fabsf(resid[5])); sumw += wgt[5];
            // sample 6
            wgt[6] = wgt[6] * expf(alpha * fabsf(resid[6])); sumw += wgt[6];
            // sample 7
            wgt[7] = wgt[7] * expf(alpha * fabsf(resid[7])); sumw += wgt[7];

            // normalise
            float inv_sum = 1.0f / sumw;
            wgt[0] *= inv_sum; wgt[1] *= inv_sum; wgt[2] *= inv_sum; wgt[3] *= inv_sum;
            wgt[4] *= inv_sum; wgt[5] *= inv_sum; wgt[6] *= inv_sum; wgt[7] *= inv_sum;
        }

        n_iter = MAX_ITER;
    }

    /* predict for a single feature value */
    float predict_one(float x) const {
        float out = 0.0f;
        // unrolled accumulation of learner contributions
        // learner 0
        float val0 = (x <= learners[0].thr) ? learners[0].left : learners[0].right;
        out += learners[0].weight * val0;
        // learner 1
        float val1 = (x <= learners[1].thr) ? learners[1].left : learners[1].right;
        out += learners[1].weight * val1;
        // learner 2
        float val2 = (x <= learners[2].thr) ? learners[2].left : learners[2].right;
        out += learners[2].weight * val2;
        return out;
    }

    /* predict on the internal test set and print */
    void demo(const float X[MAX_SAMPLES]) const {
        std::cout << std::fixed << std::setprecision(4);
        for (int i = 0; i < n_samples; ++i) {
            float yhat = predict_one(X[i]);
            std::cout << "x = " << X[i] << " -> ŷ = " << yhat << '\n';
        }
    }
};

int main() {
    /* deterministic training data (single feature) with negative, zero, and positive values */
    float X[MAX_SAMPLES] = { -1.0f, -0.5f, 0.0f, 0.3f,
                              0.8f, 1.2f, 2.0f, 3.0f };
    float Y[MAX_SAMPLES] = { -2.0f, -1.0f, 0.0f, 0.5f,
                              1.5f, 2.5f, 3.0f, 4.0f };

    AdaBoostReg model;
    model.fit(X, Y, MAX_SAMPLES);
    std::cout << "AdaBoost Regressor predictions:\n";
    model.demo(X);
    return 0;
}
