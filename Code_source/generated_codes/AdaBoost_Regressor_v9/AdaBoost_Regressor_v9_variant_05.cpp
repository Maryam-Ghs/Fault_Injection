#include <iostream>
#include <cmath>
#include <algorithm>

/* LLM input variant 5: duplicate-heavy */

int main()
{
    // ------------------------------------------------------------
    // 1. Deterministic training data (single feature, regression)
    // ------------------------------------------------------------
    int sampleCount = 6;            // number of training points
    int weakCount   = 3;            // number of weak regressors

    // Allocate feature vector, target vector and sample weights on the heap
    float* feature = new float[sampleCount];
    float* target  = new float[sampleCount];
    float* weight  = new float[sampleCount];

    // Fill with a duplicate‑heavy pattern (no cin, no random)
    feature[0] = 1.0f;  target[0] = 2.0f;
    feature[1] = 1.0f;  target[1] = 2.5f;
    feature[2] = 2.0f;  target[2] = 3.0f;
    feature[3] = 2.0f;  target[3] = 3.5f;
    feature[4] = 3.0f;  target[4] = 4.0f;
    feature[5] = 3.0f;  target[5] = 4.5f;

    // Initialise uniform weights: w_i = 1 / N
    int i = 0;
    while (i < sampleCount)
    {
        weight[i] = 1.0f / static_cast<float>(sampleCount);
        ++i;
    }

    // ------------------------------------------------------------
    // 2. Storage for weak regressors
    // ------------------------------------------------------------
    float* thresh    = new float[weakCount];   // split point
    float* leftVal   = new float[weakCount];   // prediction for x <= thresh
    float* rightVal  = new float[weakCount];   // prediction for x > thresh
    float* alpha     = new float[weakCount];   // learner weight

    // ------------------------------------------------------------
    // 3. AdaBoost.R2 training loop (verbose, step‑by‑step)
    // ------------------------------------------------------------
    int m = 0;
    while (m < weakCount)
    {
        // ---- 3.1 Find the best decision stump for current weights ----
        // Sort a copy of the feature values to generate candidate thresholds
        float* sortedX = new float[sampleCount];
        int j = 0;
        while (j < sampleCount)
        {
            sortedX[j] = feature[j];
            ++j;
        }
        // Simple bubble sort (deterministic, no std::sort)
        int a = 0;
        while (a < sampleCount - 1)
        {
            int b = 0;
            while (b < sampleCount - a - 1)
            {
                if (sortedX[b] > sortedX[b + 1])
                {
                    float tmp = sortedX[b];
                    sortedX[b] = sortedX[b + 1];
                    sortedX[b + 1] = tmp;
                }
                ++b;
            }
            ++a;
        }

        // Candidate thresholds are mid‑points between consecutive sorted values
        int candCnt = sampleCount - 1;
        float* candThresh = new float[candCnt];
        int c = 0;
        while (c < candCnt)
        {
            // (x_i + x_{i+1}) / 2  ==  0.5f * (x_i + x_{i+1})
            candThresh[c] = 0.5f * (sortedX[c] + sortedX[c + 1]);
            ++c;
        }

        // Search for the threshold that minimises weighted absolute error
        float bestErr   = 1e30f;
        float bestTh    = 0.0f;
        float bestLVal  = 0.0f;
        float bestRVal  = 0.0f;

        int tIdx = 0;
        while (tIdx < candCnt)
        {
            float th = candThresh[tIdx];

            // Compute weighted means for left and right partitions
            float sumWL = 0.0f, sumWR = 0.0f;
            float sumYL = 0.0f, sumYR = 0.0f;
            int k = 0;
            while (k < sampleCount)
            {
                if (feature[k] <= th)
                {
                    sumWL = sumWL + weight[k];
                    sumYL = sumYL + weight[k] * target[k];
                }
                else
                {
                    sumWR = sumWR + weight[k];
                    sumYR = sumYR + weight[k] * target[k];
                }
                ++k;
            }

            // Avoid division by zero
            float leftMean  = (sumWL > 0.0f) ? (sumYL / sumWL) : 0.0f;
            float rightMean = (sumWR > 0.0f) ? (sumYR / sumWR) : 0.0f;

            // Weighted absolute error for this split
            float err = 0.0f;
            k = 0;
            while (k < sampleCount)
            {
                float pred = (feature[k] <= th) ? leftMean : rightMean;
                err = err + weight[k] * std::fabs(target[k] - pred);
                ++k;
            }

            // Keep the best split
            if (err < bestErr)
            {
                bestErr  = err;
                bestTh   = th;
                bestLVal = leftMean;
                bestRVal = rightMean;
            }
            ++tIdx;
        }

        // Store the chosen weak regressor
        thresh[m]  = bestTh;
        leftVal[m] = bestLVal;
        rightVal[m]= bestRVal;

        delete[] sortedX;
        delete[] candThresh;

        // ---- 3.2 Compute learner weight (alpha) ----
        // AdaBoost.R2 uses: beta = err / (2 - err), alpha = log(1/beta)
        // err must be in (0,1); clamp to avoid division by zero
        float eps = 1e-6f;
        float errClamped = (bestErr > 1.0f - eps) ? (1.0f - eps) : bestErr;
        errClamped       = (errClamped < eps) ? eps : errClamped;
        float beta = errClamped / (2.0f - errClamped);
        alpha[m] = std::log(1.0f / beta);          // log returns double -> cast

        // ---- 3.3 Update sample weights according to AdaBoost.R2 ----
        // First compute the loss for each sample with the new weak learner
        float maxLoss = 0.0f;
        float* loss = new float[sampleCount];
        i = 0;
        while (i < sampleCount)
        {
            float pred_i = (feature[i] <= bestTh) ? bestLVal : bestRVal;
            loss[i] = std::fabs(target[i] - pred_i);
            // keep track of maximal loss (for normalisation)
            maxLoss = (loss[i] > maxLoss) ? loss[i] : maxLoss;
            ++i;
        }

        // Normalise losses to [0,1]
        i = 0;
        while (i < sampleCount)
        {
            loss[i] = (maxLoss > 0.0f) ? (loss[i] / maxLoss) : 0.0f;
            ++i;
        }

        // Update weights: w_i = w_i * beta^{(1 - L_i)}
        i = 0;
        while (i < sampleCount)
        {
            // exponentiation via std::pow (double) -> cast back to float
            float factor = std::pow(beta, 1.0f - loss[i]);
            weight[i] = weight[i] * factor;
            ++i;
        }
        delete[] loss;

        // Normalise weights so they sum to 1
        float sumW = 0.0f;
        i = 0;
        while (i < sampleCount)
        {
            sumW = sumW + weight[i];
            ++i;
        }
        i = 0;
        while (i < sampleCount)
        {
            // weight_i = weight_i / sumW   ===   weight_i * (1 / sumW)
            weight[i] = weight[i] * (1.0f / sumW);
            ++i;
        }

        ++m;   // next weak learner
    }

    // ------------------------------------------------------------
    // 4. Combine weak learners to obtain final prediction
    // ------------------------------------------------------------
    float* finalPred = new float[sampleCount];
    i = 0;
    while (i < sampleCount)
    {
        float agg = 0.0f;
        int    t = 0;
        while (t < weakCount)
        {
            float pred = (feature[i] <= thresh[t]) ? leftVal[t] : rightVal[t];
            // aggregation: sum(alpha * pred)
            agg = agg + alpha[t] * pred;
            ++t;
        }
        // normalise by total alpha
        float sumAlpha = 0.0f;
        t = 0;
        while (t < weakCount)
        {
            sumAlpha = sumAlpha + alpha[t];
            ++t;
        }
        finalPred[i] = agg / sumAlpha;
        ++i;
    }

    // ------------------------------------------------------------
    // 5. Print original targets and AdaBoost predictions
    // ------------------------------------------------------------
    std::cout << "Index   X   Target   AdaBoost_Prediction\n";
    i = 0;
    while (i < sampleCount)
    {
        std::cout << i << "   "
                  << feature[i] << "   "
                  << target[i] << "   "
                  << finalPred[i] << "\n";
        ++i;
    }

    // ------------------------------------------------------------
    // 6. Clean‑up heap memory
    // ------------------------------------------------------------
    delete[] feature;
    delete[] target;
    delete[] weight;
    delete[] thresh;
    delete[] leftVal;
    delete[] rightVal;
    delete[] alpha;
    delete[] finalPred;

    return 0;
}
