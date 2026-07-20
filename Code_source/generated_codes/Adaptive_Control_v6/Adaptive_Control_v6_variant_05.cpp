#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */

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
        theta[0] = 0.5f;
        theta[1] = 0.0f;
        gamma   = 0.02f;

        // deterministic duplicate-heavy reference trajectory
        for (int idx = 0; idx < N; ++idx)
        {
            // repeat pattern: 0.5, -0.5, 0.5, -0.5, ...
            ref[idx] = (idx % 2 == 0) ? 0.5f : -0.5f;
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
            // duplicate-heavy noise: always zero
            float noise = 0.0f;
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
    // medium sized deterministic array length
    int horizon = 30;          // fits stack arrays comfortably
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
