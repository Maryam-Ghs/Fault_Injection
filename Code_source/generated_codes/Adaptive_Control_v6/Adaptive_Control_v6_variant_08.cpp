/* LLM input variant 8: sparse-skewed */
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
        theta[0] = 0.5f;
        theta[1] = 0.0f;
        gamma   = 0.02f;

        // random seed (used for noise only)
        std::srand( static_cast<unsigned>( std::time(0) ) );

        // generate sparse, skewed reference and zero initial plant output
        for (int idx = 0; idx < N; ++idx)
        {
            // First few values form a small cluster
            if (idx < 5)
                ref[idx] = (idx + 1) * 0.2f;          // 0.2, 0.4, 0.6, 0.8, 1.0
            // Occasionally a negative spike at regular intervals
            else if (idx % 20 == 0)
                ref[idx] = -0.7f;
            else
                ref[idx] = 0.0f;                       // mostly zero (sparse)

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
            float noise = ( std::rand() % 20 - 10 ) * 0.001f; // [-0.01,0.01]
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
    // larger horizon to showcase sparsity while staying within static arrays
    int horizon = 80;          // fits stack arrays comfortably
    AdaptiveCtrl ctrl(horizon);
    ctrl.simulate();
    ctrl.report();
    return 0;
}
