/* LLM input variant 3: zeros-and-ones */
// A2C implementation – version #9
// Everything is inside main(), using only float and int types.

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

int main() {
    // ------------------------------------------------------------
    // Hyper‑parameters (all float / int)
    // ------------------------------------------------------------
    int      num_episodes = 2;           // how many episodes to run
    int      max_steps    = 4;           // length of each trajectory
    int      dim_state    = 3;           // dimensionality of the state vector
    int      dim_action   = 2;           // number of discrete actions
    float    gamma        = 0.99f;      // discount factor
    float    learning_rate= 0.01f;      // simple SGD step size

    // ------------------------------------------------------------
    // Random seed for reproducibility (kept but not used)
    // ------------------------------------------------------------
    std::srand( static_cast<unsigned>( std::time( nullptr ) ) );

    // ------------------------------------------------------------
    // Policy parameters: linear mapping state -> logits (dim_action)
    // Value parameters : linear mapping state -> scalar value
    // ------------------------------------------------------------
    std::vector<float> w_policy( dim_state * dim_action, 0.0f );
    std::vector<float> w_value ( dim_state, 0.0f );

    // ------------------------------------------------------------
    // Main training loop – everything lives inside main()
    // ------------------------------------------------------------
    int episode = 0;
    while ( episode < num_episodes ) {

        // --------------------------------------------------------
        // 1) Generate a deterministic trajectory (states, actions, rewards)
        // --------------------------------------------------------
        std::vector< std::vector<float> > traj_state( max_steps, std::vector<float>( dim_state, 0.0f ) );
        std::vector<int>                 traj_action( max_steps, 0 );
        std::vector<float>               traj_reward( max_steps, 0.0f );

        int step = 0;
        while ( step < max_steps ) {
            // fill state with a pattern of 0s and 1s
            int i = 0;
            while ( i < dim_state ) {
                traj_state[step][i] = ( (step + i) % 2 == 0 ) ? 0.0f : 1.0f;
                ++i;
            }

            // deterministic action: alternating 0 and 1
            traj_action[step] = step % dim_action;

            // deterministic reward: 1 on even steps, 0 on odd steps
            traj_reward[step] = ( step % 2 == 0 ) ? 1.0f : 0.0f;

            ++step;
        }

        // --------------------------------------------------------
        // 2) Compute discounted returns G_t for every time step
        // --------------------------------------------------------
        std::vector<float> ret( max_steps, 0.0f );
        float acc = 0.0f;
        int t = max_steps - 1;
        while ( t >= 0 ) {
            acc = traj_reward[t] + gamma * acc;
            ret[t] = acc;
            --t;
        }

        // --------------------------------------------------------
        // 3) Estimate state values V(s_t) using current value weights
        // --------------------------------------------------------
        std::vector<float> val_est( max_steps, 0.0f );
        t = 0;
        while ( t < max_steps ) {
            float dot = 0.0f;
            int j = 0;
            while ( j < dim_state ) {
                dot += w_value[j] * traj_state[t][j];
                ++j;
            }
            val_est[t] = dot;
            ++t;
        }

        // --------------------------------------------------------
        // 4) Compute advantages A_t = G_t - V(s_t)
        // --------------------------------------------------------
        std::vector<float> adv( max_steps, 0.0f );
        t = 0;
        while ( t < max_steps ) {
            adv[t] = ret[t] - val_est[t];
            ++t;
        }

        // --------------------------------------------------------
        // 5) Accumulate gradients for policy and value networks
        // --------------------------------------------------------
        // Initialise gradient accumulators with zeros
        std::vector<float> grad_pi( dim_state * dim_action, 0.0f );
        std::vector<float> grad_v ( dim_state, 0.0f );

        float total_loss = 0.0f;

        t = 0;
        while ( t < max_steps ) {
            // ----- 5.1) Forward pass through policy network -----
            // compute logits = w_policy * state
            std::vector<float> logits( dim_action, 0.0f );
            int a = 0;
            while ( a < dim_action ) {
                float dot = 0.0f;
                int s = 0;
                while ( s < dim_state ) {
                    dot += w_policy[ a * dim_state + s ] * traj_state[t][s];
                    ++s;
                }
                logits[a] = dot;
                ++a;
            }

            // ----- 5.2) Softmax to obtain action probabilities -----
            // find max for numerical stability
            float max_logit = logits[0];
            a = 1;
            while ( a < dim_action ) {
                if ( logits[a] > max_logit ) max_logit = logits[a];
                ++a;
            }

            // exponentiate shifted logits
            std::vector<float> exp_shifted( dim_action, 0.0f );
            float sum_exp = 0.0f;
            a = 0;
            while ( a < dim_action ) {
                float e = std::exp( logits[a] - max_logit );
                exp_shifted[a] = e;
                sum_exp += e;
                ++a;
            }

            // probabilities
            std::vector<float> probs( dim_action, 0.0f );
            a = 0;
            while ( a < dim_action ) {
                probs[a] = exp_shifted[a] / sum_exp;
                ++a;
            }

            // ----- 5.3) Compute loss components -----
            int act = traj_action[t];
            float prob_act = probs[act];
            // avoid log(0) by clipping
            if ( prob_act < 1e-6f ) prob_act = 1e-6f;
            float logp = std::log( prob_act );

            // policy loss = -logπ(a|s) * A
            float loss_pi = -logp * adv[t];

            // value loss = 0.5 * (A)^2
            float loss_v = 0.5f * adv[t] * adv[t];

            // total loss for reporting
            total_loss += loss_pi + loss_v;

            // ----- 5.4) Back‑propagate policy gradient -----
            // gradient of loss w.r.t logits = (π - 1_{a}) * A
            std::vector<float> dlogits( dim_action, 0.0f );
            a = 0;
            while ( a < dim_action ) {
                float indicator = ( a == act ) ? 1.0f : 0.0f;
                dlogits[a] = ( probs[a] - indicator ) * adv[t];
                ++a;
            }

            // accumulate policy weight gradients
            a = 0;
            while ( a < dim_action ) {
                int s = 0;
                while ( s < dim_state ) {
                    grad_pi[ a * dim_state + s ] += dlogits[a] * traj_state[t][s];
                    ++s;
                }
                ++a;
            }

            // ----- 5.5) Back‑propagate value gradient -----
            // gradient of 0.5 * A^2 w.r.t value = -A
            float dvalue = -adv[t];
            int s = 0;
            while ( s < dim_state ) {
                grad_v[s] += dvalue * traj_state[t][s];
                ++s;
            }

            ++t;
        } // end of trajectory loop

        // --------------------------------------------------------
        // 6) Perform a simple SGD update
        // --------------------------------------------------------
        int idx = 0;
        while ( idx < dim_state * dim_action ) {
            w_policy[idx] -= learning_rate * ( grad_pi[idx] / static_cast<float>( max_steps ) );
            ++idx;
        }

        idx = 0;
        while ( idx < dim_state ) {
            w_value[idx] -= learning_rate * ( grad_v[idx] / static_cast<float>( max_steps ) );
            ++idx;
        }

        // --------------------------------------------------------
        // 7) Report episode statistics
        // --------------------------------------------------------
        std::cout << "Episode " << episode << " | Avg loss: " << ( total_loss / static_cast<float>( max_steps ) ) << std::endl;

        ++episode;
    } // end of training loop

    // ------------------------------------------------------------
    // Final policy weights (optional display)
    // ------------------------------------------------------------
    std::cout << "\nFinal policy weights (flattened):" << std::endl;
    for ( size_t i = 0; i < w_policy.size(); ++i ) {
        std::cout << w_policy[i] << ( i + 1 == w_policy.size() ? "\n" : " " );
    }

    std::cout << "\nFinal value weights:" << std::endl;
    for ( size_t i = 0; i < w_value.size(); ++i ) {
        std::cout << w_value[i] << ( i + 1 == w_value.size() ? "\n" : " " );
    }

    return 0;
}
