#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

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

        // random seed
        std::srand( static_cast<unsigned>( std::time(0) ) );

        // generate random reference and initial plant output
        int idx = 0;
        while (idx < N)
        {
            ref[idx] = ( std::rand() % 200 - 100 ) * 0.01f;   // [-1,1]
            out[idx] = 0.0f;
            ++idx;
        }
    }

    void simulate()
    {
        int k = 0;
        while (k < N)
        {
            // control law: u = theta0 * ref + theta1 * previous output
            // reordered arithmetic to reduce temporaries
            inp[k] = (theta[0] * ref[k]) + (theta[1] * (k ? out[k-1] : 0.0f));

            // simple plant: y(k+1) = 0.8*y(k) + u(k) + noise
            // noise is small random value
            float noise = ( std::rand() % 20 - 10 ) * 0.001f; // [-0.01,0.01]
            out[k] = (0.8f * (k ? out[k-1] : 0.0f)) + inp[k] + noise;

            // tracking error
            float err = out[k] - ref[k];

            // adaptation law (gradient descent)
            // theta = theta - gamma * err * phi
            // phi = [ref, previous output]
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
    // large stress horizon, still within static array limits
    int horizon = 95;          // near the 100‑element stack capacity
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
