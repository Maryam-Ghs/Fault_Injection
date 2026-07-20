/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <iomanip>

class AdaptiveCtrl {
public:
    AdaptiveCtrl(int sz) : dim(sz) {
        theta = new float[dim];
        phi   = new float[dim];
        for (int i = 0; i < dim; ++i) {
            theta[i] = 0.0f;
            phi[i]   = 0.0f;
        }
    }

    ~AdaptiveCtrl() {
        delete[] theta;
        delete[] phi;
    }

    void adapt(float ref, float out, float lr) {
        float err = ref - out;
        float tmp_phi = out;
        phi[0] = tmp_phi;
        float tmp_u = 0.0f;
        tmp_u = -theta[0] * out;
        float ctrl = tmp_u + ref;
        float tmp_adapt = lr * err * phi[0];
        float delta = tmp_adapt;
        float new_theta = theta[0] + delta;
        theta[0] = new_theta;
        last_u = ctrl;
    }

    float getControl() const { return last_u; }

private:
    int   dim;
    float *theta;
    float *phi;
    float  last_u;
};

int main() {
    const int N = 10;
    float *refSig = new float[N];
    float *plantY = new float[N];
    float *ctrlU  = new float[N];

    for (int k = 0; k < N; ++k) {
        refSig[k] = (k % 2 == 0) ? 0.0f : 1.0f; // alternating 0 and 1
    }

    for (int k = 0; k < N; ++k) {
        plantY[k] = 0.0f;
    }

    AdaptiveCtrl ac(1);
    float learnRate = 1.0f; // identity gain

    for (int t = 0; t < N; ++t) {
        ac.adapt(refSig[t], plantY[t], learnRate);
        float u_now = ac.getControl();
        ctrlU[t] = u_now;
        float pred = 0.8f * plantY[t];
        float incr = 0.2f * u_now;
        float y_next = pred + incr;
        if (t + 1 < N) {
            plantY[t + 1] = y_next;
        }
    }

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Step  Ref  Out  Ctrl\n";
    for (int i = 0; i < N; ++i) {
        std::cout << i << "   " << refSig[i] << "  " << plantY[i] << "  " << ctrlU[i] << "\n";
    }

    delete[] refSig;
    delete[] plantY;
    delete[] ctrlU;

    return 0;
}
