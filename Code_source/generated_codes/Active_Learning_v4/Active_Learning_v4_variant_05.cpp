#include <iostream>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

class Learner {
    int  nFeat;          // number of features
    int  nPool;          // total samples
    int *idxLab;         // indices of labelled samples
    int *idxUnl;         // indices of unlabelled samples
    int  cntLab;         // how many labelled
    int  cntUnl;         // how many unlabelled
    float *feat;         // flattened feature matrix (nPool * nFeat)
    float *trueLbl;      // true labels (+1 / -1)
    float *estLbl;       // current estimated labels for pool
    float *weight;       // perceptron weights (nFeat)

public:
    Learner(int samples, int features)
        : nFeat(features), nPool(samples), cntLab(0), cntUnl(samples) {
        feat = new float[nPool * nFeat];
        trueLbl = new float[nPool];
        estLbl = new float[nPool];
        weight = new float[nFeat];
        idxLab = new int[nPool];
        idxUnl = new int[nPool];
        for (int i = 0; i < nFeat; ++i) weight[i] = 0.0f;
        for (int i = 0; i < nPool; ++i) {
            idxUnl[i] = i;
            estLbl[i] = 0.0f;
        }
    }

    ~Learner() {
        delete[] feat; delete[] trueLbl; delete[] estLbl;
        delete[] weight; delete[] idxLab; delete[] idxUnl;
    }

    void loadData(const float *src, const float *lbl) {
        for (int i = 0; i < nPool * nFeat; ++i) feat[i] = src[i];
        for (int i = 0; i < nPool; ++i) trueLbl[i] = lbl[i];
    }

    // train perceptron on currently labelled set (single‑epoch)
    void train() {
        for (int i = 0; i < cntLab; ++i) {
            int id = idxLab[i];
            float dot = 0.0f;
            for (int j = 0; j < nFeat; ++j) dot += weight[j] * feat[id * nFeat + j];
            float pred = dot >= 0.0f ? 1.0f : -1.0f;
            float err = trueLbl[id] - pred;
            for (int j = 0; j < nFeat; ++j) weight[j] += err * feat[id * nFeat + j];
        }
    }

    // compute absolute margins for all unlabelled samples
    float uncertainty(int id) {
        float dot = 0.0f;
        for (int j = 0; j < nFeat; ++j) dot += weight[j] * feat[id * nFeat + j];
        return std::fabs(dot);
    }

    // pick the most uncertain sample (smallest margin)
    int select() {
        int best = idxUnl[0];
        float bestVal = uncertainty(best);
        for (int i = 1; i < cntUnl; ++i) {
            int cur = idxUnl[i];
            float curVal = uncertainty(cur);
            best = curVal < bestVal ? cur : best;
            bestVal = curVal < bestVal ? curVal : bestVal;
        }
        return best;
    }

    // remove id from unlabelled list (swap‑erase)
    void discard(int id) {
        for (int i = 0; i < cntUnl; ++i) {
            if (idxUnl[i] == id) {
                idxUnl[i] = idxUnl[--cntUnl];
                break;
            }
        }
    }

    // run active learning until pool exhausted
    void run(int start) {
        // seed with first *start* samples
        for (int i = 0; i < start; ++i) {
            idxLab[cntLab++] = idxUnl[i];
        }
        cntUnl -= start;
        for (int i = 0; i < cntUnl; ++i) idxUnl[i] = idxUnl[i + start];
        train();

        while (cntUnl) {
            int cand = select();
            idxLab[cntLab++] = cand;
            discard(cand);
            train();
        }
    }

    void report() {
        std::cout << "Final weights:";
        for (int i = 0; i < nFeat; ++i) std::cout << ' ' << weight[i];
        std::cout << "\nPredictions on whole pool:\n";
        for (int i = 0; i < nPool; ++i) {
            float dot = 0.0f;
            for (int j = 0; j < nFeat; ++j) dot += weight[j] * feat[i * nFeat + j];
            float pred = dot >= 0.0f ? 1.0f : -1.0f;
            std::cout << "sample " << i << ": true " << trueLbl[i]
                      << " pred " << pred << "\n";
        }
    }
};

int main() {
    // duplicated dataset: 8 samples, 2 features each
    const float rawFeat[16] = {
        1.0f,  1.0f,   // sample 0
        1.0f,  1.0f,   // sample 1 (duplicate of 0)
        -1.0f, -1.0f,  // sample 2
        -1.0f, -1.0f,  // sample 3 (duplicate of 2)
        0.0f,  0.0f,   // sample 4
        0.0f,  0.0f,   // sample 5 (duplicate of 4)
        1.0f,  1.0f,   // sample 6 (another duplicate of 0)
        -1.0f, -1.0f   // sample 7 (another duplicate of 2)
    };
    const float rawLbl[8] = {
        1.0f, 1.0f,   // labels for duplicates of (1,1)
        -1.0f, -1.0f, // labels for duplicates of (-1,-1)
        1.0f, 1.0f,   // labels for duplicates of (0,0) (treated as positive)
        -1.0f, -1.0f  // more negatives
    };

    Learner agent(8, 2);
    agent.loadData(rawFeat, rawLbl);
    agent.run(2);          // start with 2 labelled points
    agent.report();
    return 0;
}
