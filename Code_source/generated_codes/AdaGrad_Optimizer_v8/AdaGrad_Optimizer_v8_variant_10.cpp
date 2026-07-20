/* LLM input variant 10: large-safe-stress */
// AdaGrad Optimizer - Version #8
// ------------------------------------------------------------
// This program creates a small synthetic linear regression
// problem, then optimizes the single weight using the AdaGrad
// algorithm. All data resides on the stack, and the code is
// deliberately verbose, showing each arithmetic step.
// ------------------------------------------------------------

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// Helper: fill arrays with pseudo‑random numbers in [0,1)
// ------------------------------------------------------------
void generate_data(int size, float xs[], float ys[])
{
    // seed the generator once
    static bool seeded = false;
    if (!seeded)
    {
        srand(42);
        seeded = true;
    }

    // generate a random slope and intercept for the hidden model
    float true_w = 2.5f;       // hidden weight
    float true_b = 0.7f;       // hidden bias (we will ignore bias in the optimizer)

    for (int i = 0; i < size; ++i)
    {
        // x_i ~ Uniform(0,1)
        float rnd_x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        xs[i] = rnd_x;

        // y_i = true_w * x_i + true_b + noise
        float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.1f;
        ys[i] = true_w * rnd_x + true_b + noise;
    }
}

// ------------------------------------------------------------
// Helper: compute gradient of the MSE loss w.r.t. weight w
// ------------------------------------------------------------
float compute_gradient(int size, const float xs[], const float ys[], float w_cur)
{
    // gradient = (1/N) * Σ ( (w*x_i - y_i) * x_i )
    float sum = 0.0f;
    for (int i = 0; i < size; ++i)
    {
        // prediction = w_cur * x_i
        float pred = w_cur * xs[i];

        // error = pred - y_i
        float err = pred - ys[i];

        // contribution = err * x_i
        float contrib = err * xs[i];

        // accumulate
        sum = sum + contrib;
    }

    // average over examples
    float grad = sum / static_cast<float>(size);
    return grad;
}

// ------------------------------------------------------------
// Helper: AdaGrad parameter update (single weight)
// ------------------------------------------------------------
void adagrad_update(float& w_param,
                    float grad_param,
                    float& accum_sq,
                    float lr,
                    float eps)
{
    // 1. square the gradient
    float grad_sq = grad_param * grad_param;

    // 2. accumulate squared gradients
    accum_sq = accum_sq + grad_sq;

    // 3. compute adjusted learning rate: lr_adj = lr / ( sqrt(accum_sq) + eps )
    float sqrt_acc = sqrtf(accum_sq);
    float denom = sqrt_acc + eps;
    float lr_adj = lr / denom;

    // 4. compute step = lr_adj * grad_param
    float step = lr_adj * grad_param;

    // 5. update weight: w_new = w_old - step
    w_param = w_param - step;
}

// ------------------------------------------------------------
// Helper: display current optimizer state
// ------------------------------------------------------------
void print_state(int epoch,
                 float w_param,
                 float grad_param,
                 float accum_sq)
{
    std::cout << "Epoch " << epoch
              << " | weight = " << w_param
              << " | grad = " << grad_param
              << " | G = " << accum_sq
              << std::endl;
}

// ------------------------------------------------------------
// Main entry point
// ------------------------------------------------------------
int main()
{
    // --------------------------------------------------------
    // 1. Problem size and data containers (stack allocation)
    // --------------------------------------------------------
    const int N = 10000;                     // larger safe stress test
    float x_vals[N];
    float y_vals[N];

    // --------------------------------------------------------
    // 2. Generate synthetic data
    // --------------------------------------------------------
    generate_data(N, x_vals, y_vals);

    // --------------------------------------------------------
    // 3. Optimizer hyper‑parameters
    // --------------------------------------------------------
    float learning_rate = 0.4f;     // base step size
    float epsilon = 1e-6f;          // numerical stability term

    // --------------------------------------------------------
    // 4. Parameter initialization
    // --------------------------------------------------------
    float w_param = 0.0f;           // start from zero
    float accum_sq = 0.0f;          // G(t) = 0 initially

    // --------------------------------------------------------
    // 5. Training loop (verbose, while‑style)
    // --------------------------------------------------------
    int epoch = 0;
    const int max_epochs = 200;    // more epochs for larger data
    while (epoch < max_epochs)
    {
        // ---- a) compute current gradient ----
        float grad_cur = compute_gradient(N, x_vals, y_vals, w_param);

        // ---- b) perform AdaGrad update ----
        adagrad_update(w_param, grad_cur, accum_sq, learning_rate, epsilon);

        // ---- c) report state ----
        print_state(epoch, w_param, grad_cur, accum_sq);

        // ---- d) advance epoch counter ----
        epoch = epoch + 1;
    }

    // --------------------------------------------------------
    // 6. Final result
    // --------------------------------------------------------
    std::cout << "\nOptimized weight after " << max_epochs
              << " epochs: " << w_param << std::endl;

    return 0;
}
