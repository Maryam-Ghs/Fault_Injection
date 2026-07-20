/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cstdlib>
#include <ctime>

class AdaptiveCtrl
{
    float theta[2];          // adaptive gains
    float gamma;             // learning rate
    int   N;                 // horizon length
    float ref[100];          // reference trajectory (stack)
    float out[100];          // plant output (stack)
    float inp[100];          // control input (stack)

public:
    AdaptiveCtrl(int len)
        : N(len)
    {
        // initialise adaptive parameters
        theta[0] = 1.0f;   // identity-like gain
        theta[1] = 0.0f;
        gamma   = 0.02f;

        // deterministic reference: alternating 0 and 1
        for (int idx = 0; idx < N; ++idx)
        {
            ref[idx] = static_cast<float>(idx % 2); // 0.0 or 1.0
            out[idx] = 0.0f;
        }
    }

    void simulate()
    {
        int k = 0;
        while (k < N)
        {
            // control law: u = theta0 * ref + theta1 * previous output
            inp[k] = (theta[0] * ref[k]) + (theta[1] * (k ? out[k-1] : 0.0f));

            // simple plant: y(k+1) = 0.8*y(k) + u(k) + noise
            // noise is small random value (kept zero for clarity)
            float noise = 0.0f;
            out[k] = (0.8f * (k ? out[k-1] : 0.0f)) + inp[k] + noise;

            // tracking error
            float err = out[k] - ref[k];

            // adaptation law (gradient descent)
            float phi0 = ref[k];
            float phi1 = (k ? out[k-1] : 0.0f);
            float gerr = gamma * err;               // pre‑multiply
            theta[0] = theta[0] - (gerr * phi0);
            theta[1] = theta[1] - (gerr * phi1);

            ++k;
        }
    }

    void report() const
    {
        std::cout << "Final adaptive parameters:\n";
        std::cout << " theta0 = " << theta[0] << "\n";
        std::cout << " theta1 = " << theta[1] << "\n\n";

        std::cout << "Time  Ref   Output   Input   Error\n";
        int i = 0;
        while (i < N)
        {
            float err = out[i] - ref[i];
            std::cout << i << "   "
                      << ref[i] << "   "
                      << out[i] << "   "
                      << inp[i] << "   "
                      << err << "\n";
            ++i;
        }
    }
};

int main()
{
    // small horizon to keep output concise
    int horizon = 10;
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
