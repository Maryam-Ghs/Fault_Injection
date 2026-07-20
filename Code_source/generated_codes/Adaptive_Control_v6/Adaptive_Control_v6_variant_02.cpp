#include <iostream>
/* LLM input variant 2: small-diverse */

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
        // initialise adaptive parameters with slightly different start values
        theta[0] = 0.4f;
        theta[1] = 0.1f;
        gamma   = 0.015f;

        // deterministic reference trajectory with varied values
        // pattern: -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, repeat
        for (int idx = 0; idx < N; ++idx)
        {
            ref[idx] = ((idx % 7) - 3) * 0.2f;   // [-0.6,0.6] step 0.2
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

            // deterministic small noise pattern
            float noise = ((k % 5) - 2) * 0.001f; // [-0.002,0.002]
            out[k] = (0.8f * (k ? out[k-1] : 0.0f)) + inp[k] + noise;

            // tracking error
            float err = out[k] - ref[k];

            // adaptation law (gradient descent)
            float phi0 = ref[k];
            float phi1 = (k ? out[k-1] : 0.0f);
            float gerr = gamma * err;
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
    // small diverse horizon length
    int horizon = 12;
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
