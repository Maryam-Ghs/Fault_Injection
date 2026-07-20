/ * LLM input variant 1: minimal-boundary * /
#include <iostream>
#include <iomanip>

class AdaptiveCtrl {
public:
    AdaptiveCtrl(int sz) : dim(sz) {
        // allocate internal buffers on the heap
        theta = new float[dim];
        phi   = new float[dim];
        // initialise to zero
        for (int i = 0; i < dim; ++i) {
            theta[i] = 0.0f;
            phi[i]   = 0.0f;
        }
    }

    ~AdaptiveCtrl() {
        delete[] theta;
        delete[] phi;
    }

    // one adaptation step
    void adapt(float ref, float out, float lr) {
        // ---- expanded multi‑step calculations ----
        // compute tracking error
        float err = ref - out;

        // build regression vector (here simply the plant output)
        float tmp_phi = out;                     // step 1
        phi[0] = tmp_phi;                        // step 2

        // compute control law using current estimate
        float tmp_u = 0.0f;                      // step 1
        tmp_u = -theta[0] * out;                 // step 2
        float ctrl = tmp_u + ref;                // step 3

        // adaptation term
        float tmp_adapt = lr * err * phi[0];     // step 1
        float delta = tmp_adapt;                 // step 2

        // update estimate
        float new_theta = theta[0] + delta;      // step 1
        theta[0] = new_theta;                    // step 2

        // store computed control for external use
        last_u = ctrl;
    }

    float getControl() const { return last_u; }

private:
    int   dim;        // size of parameter vector (always 1 here)
    float *theta;     // estimated parameters
    float *phi;       // regression vector
    float  last_u;    // most recent control signal
};

int main() {
    // -------------------------------------------------
    // small predefined data sets (heap allocated)
    // -------------------------------------------------
    const int N = 2;                       // minimal number of simulation steps
    float *refSig = new float[N];
    float *plantY = new float[N];
    float *ctrlU  = new float[N];

    // populate reference signal (step change from 0 to 1)
    for (int k = 0; k < N; ++k) {
        refSig[k] = (k < 1) ? 0.0f : 1.0f;
    }

    // initialise plant output to zero
    for (int k = 0; k < N; ++k) {
        plantY[k] = 0.0f;
    }

    // -------------------------------------------------
    // Adaptive control simulation
    // -------------------------------------------------
    AdaptiveCtrl ac(1);                    // 1‑dimensional parameter vector
    float learnRate = 0.05f;               // adaptation gain

    // simulation loop (branch‑minimized: single for‑loop)
    for (int t = 0; t < N; ++t) {
        // plant model: first‑order system y(k+1) = 0.8*y(k) + 0.2*u(k)
        // compute control using current estimates
        ac.adapt(refSig[t], plantY[t], learnRate);
        float u_now = ac.getControl();

        // store control
        ctrlU[t] = u_now;

        // advance plant (one‑step Euler discretisation)
        float pred = 0.8f * plantY[t];
        float incr = 0.2f * u_now;
        float y_next = pred + incr;        // expanded multi‑step
        if (t + 1 < N) {
            plantY[t + 1] = y_next;
        }
    }

    // -------------------------------------------------
    // Output results
    // -------------------------------------------------
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Step  Ref  Out  Ctrl\n";
    for (int i = 0; i < N; ++i) {
        std::cout << i << "   " << refSig[i] << "  " << plantY[i] << "  " << ctrlU[i] << "\n";
    }

    // clean up heap memory
    delete[] refSig;
    delete[] plantY;
    delete[] ctrlU;

    return 0;
}
