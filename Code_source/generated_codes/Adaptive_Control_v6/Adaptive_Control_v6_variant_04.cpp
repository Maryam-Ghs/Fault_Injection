#include <iostream>

/* LLM input variant 4: signed-extremes */

class AdaptiveCtrl
{
    float theta[2];          // adaptive gains
    float gamma;            // learning rate
    int   N;                // horizon length
    float ref[100];         // reference trajectory (stack)
    float out[100];         // plant output (stack)
    float inp[100];         // control input (stack)

public:
    AdaptiveCtrl(int len)
        : N(len)
    {
        // initialise adaptive parameters
        theta[0] = 0.5f;
        theta[1] = 0.0f;
        gamma   = 0.02f;

        // deterministic mixed-sign reference trajectory
        const float vals[5] = { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
        for (int idx = 0; idx < N; ++idx)
        {
            ref[idx] = vals[idx % 5];
            out[idx] = 0.0f;
        }
    }

    void simulate()
    {
        const float noiseVals[3] = { -0.01f, 0.0f, 0.01f };
        int k = 0;
        while (k < N)
        {
            // control law: u = theta0 * ref + theta1 * previous output
            inp[k] = (theta[0] * ref[k]) + (theta[1] * (k ? out[k-1] : 0.0f));

            // simple plant: y(k+1) = 0.8*y(k) + u(k) + noise
            float noise = noiseVals[k % 3];
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
        for (int i = 0; i < N; ++i)
        {
            float err = out[i] - ref[i];
            std::cout << i << "   "
                      << ref[i] << "   "
                      << out[i] << "   "
                      << inp[i] << "   "
                      << err << "\n";
        }
    }
};

int main()
{
    // medium sized deterministic horizon
    int horizon = 25;          // fits stack arrays comfortably
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
