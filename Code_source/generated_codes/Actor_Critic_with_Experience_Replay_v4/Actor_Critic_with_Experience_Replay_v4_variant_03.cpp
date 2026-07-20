/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* -------------------------------------------------------------------------- */
/*                         Hyper‑parameters (stack‑allocated)                 */
/* -------------------------------------------------------------------------- */
int MAX_BUFFER = 200;          // maximum number of stored transitions
int STATE_DIM   = 3;           // dimensionality of the state vector
int ACTION_DIM  = 2;           // number of possible discrete actions
int BATCH_SIZE  = 1;           // size of a sampled batch (minimal)
int EPISODES    = 5;           // number of training episodes (reduced)
int STEPS_PER_EP = 3;          // steps per episode (small to test edge cases)

float GAMMA    = 1.0f;         // discount factor – edge case (no discount)
float ALPHA_A  = 1.0f;         // actor learning rate – identity (full learning)
float ALPHA_C  = 1.0f;         // critic learning rate – identity (full learning)

/* -------------------------------------------------------------------------- */
/*                         Experience‑Replay Buffer (stack)                     */
/* -------------------------------------------------------------------------- */
float   buf_state[200][3];
int     buf_action[200];
float   buf_reward[200];
float   buf_next_state[200][3];
int     buf_done[200];
int     buf_size = 0;

/* -------------------------------------------------------------------------- */
/*                         Utility Functions                                    */
/* -------------------------------------------------------------------------- */

/* Fill an array with small random numbers in the range [-0.1, 0.1] */
void random_initialize(float *arr, int len)
{
    int i = 0;
    while (i < len)
    {
        float r = (float)std::rand() / (float)RAND_MAX;    // [0,1]
        r = r * 0.2f - 0.1f;                               // [-0.1,0.1]
        arr[i] = r;
        i = i + 1;
    }
}

/* Compute dot product of two vectors (length = n) */
float dot_product(const float *a, const float *b, int n)
{
    float result = 0.0f;
    int   i = 0;
    while (i < n)
    {
        result = result + a[i] * b[i];
        i = i + 1;
    }
    return result;
}

/* -------------------------------------------------------------------------- */
/*                         Actor‑Critic Core Functions                           */
/* -------------------------------------------------------------------------- */

/* Initialise actor and critic weight tables */
void initialise_weights(float *actor_w, float *critic_w)
{
    random_initialize(actor_w, STATE_DIM * ACTION_DIM);
    random_initialize(critic_w, STATE_DIM);
}

/* Policy: choose action with highest preference (deterministic) */
int choose_action(const float *actor_w, const float *state)
{
    /* Compute preferences for each action */
    float pref0 = 0.0f;
    float pref1 = 0.0f;
    int   i = 0;
    while (i < STATE_DIM)
    {
        pref0 = pref0 + actor_w[i] * state[i];
        pref1 = pref1 + actor_w[STATE_DIM + i] * state[i];
        i = i + 1;
    }
    /* Return index of larger (or equal) preference */
    if (pref0 >= pref1)
        return 0;
    else
        return 1;
}

/* Value function: linear approximation V(s) = w · s */
float evaluate_state(const float *critic_w, const float *state)
{
    return dot_product(critic_w, state, STATE_DIM);
}

/* Store a transition into the replay buffer (simple FIFO) */
void store_transition(const float *s, int a, float r,
                      const float *s_next, int d)
{
    if (buf_size < MAX_BUFFER)
    {
        int idx = buf_size;
        int i = 0;
        while (i < STATE_DIM)
        {
            buf_state[idx][i]     = s[i];
            buf_next_state[idx][i]= s_next[i];
            i = i + 1;
        }
        buf_action[idx] = a;
        buf_reward[idx] = r;
        buf_done[idx]   = d;
        buf_size = buf_size + 1;
    }
    else
    {
        /* Overwrite oldest entry (circular buffer) */
        int idx = buf_size % MAX_BUFFER;
        int i = 0;
        while (i < STATE_DIM)
        {
            buf_state[idx][i]     = s[i];
            buf_next_state[idx][i]= s_next[i];
            i = i + 1;
        }
        buf_action[idx] = a;
        buf_reward[idx] = r;
        buf_done[idx]   = d;
        buf_size = buf_size + 1;   // keep growing for modulo logic
    }
}

/* Sample a batch of indices uniformly at random */
void sample_batch(int *indices, int batch_len)
{
    int i = 0;
    while (i < batch_len)
    {
        int rnd = std::rand() % (buf_size < MAX_BUFFER ? buf_size : MAX_BUFFER);
        indices[i] = rnd;
        i = i + 1;
    }
}

/* Perform one training step using a sampled batch */
void train_step(float *actor_w, float *critic_w)
{
    if (buf_size == 0) return;                     // nothing to train on

    int batch_idx[16];
    sample_batch(batch_idx, BATCH_SIZE);

    int b = 0;
    while (b < BATCH_SIZE)
    {
        int id = batch_idx[b];

        /* ---------- Extract transition ---------- */
        float *s      = buf_state[id];
        int    a      = buf_action[id];
        float  r      = buf_reward[id];
        float *s_next = buf_next_state[id];
        int    done   = buf_done[id];

        /* ---------- Critic update (TD‑error) ---------- */
        float v_curr = evaluate_state(critic_w, s);
        float v_next = evaluate_state(critic_w, s_next);
        /* TD‑error δ = r + γ·v_next – v_curr  (reordered) */
        float delta = r - v_curr + GAMMA * v_next;
        /* Critic weight update: w ← w + α_c·δ·s  (reordered multiplication) */
        int i = 0;
        while (i < STATE_DIM)
        {
            critic_w[i] = critic_w[i] + ALPHA_C * delta * s[i];
            i = i + 1;
        }

        /* ---------- Actor update (policy gradient) ---------- */
        /* Compute preference for chosen action */
        float pref = 0.0f;
        i = 0;
        while (i < STATE_DIM)
        {
            pref = pref + actor_w[a * STATE_DIM + i] * s[i];
            i = i + 1;
        }
        /* Gradient ascent: w_a ← w_a + α_a·δ·s */
        i = 0;
        while (i < STATE_DIM)
        {
            actor_w[a * STATE_DIM + i] = actor_w[a * STATE_DIM + i] + ALPHA_A * delta * s[i];
            i = i + 1;
        }

        b = b + 1;
    }
}

/* -------------------------------------------------------------------------- */
/*                         Synthetic Environment (edge cases)                  */
/* -------------------------------------------------------------------------- */

/* Generate a deterministic “zero‑one” state vector */
void generate_state(float *state)
{
    int i = 0;
    while (i < STATE_DIM)
    {
        /* First element 1, others 0 (identity‑like) */
        state[i] = (i == 0) ? 1.0f : 0.0f;
        i = i + 1;
    }
}

/* Simulate reward: 0 for non‑terminal, 1 for terminal step */
float compute_reward(int step, int done)
{
    if (done == 1)
        return 1.0f;                 // reward at terminal state
    else
        return 0.0f;                 // no reward otherwise
}

/* -------------------------------------------------------------------------- */
/*                         Main Training Loop                                   */
/* -------------------------------------------------------------------------- */
int main()
{
    std::srand((unsigned)std::time(0));   // seed RNG (cast to unsigned is allowed for srand)

    /* Allocate weight tables on the stack */
    float actor_weights[6];   // STATE_DIM * ACTION_DIM = 3*2 = 6
    float critic_weights[3];  // STATE_DIM = 3

    initialise_weights(actor_weights, critic_weights);

    int ep = 0;
    while (ep < EPISODES)
    {
        /* ---------- Initialise episode ---------- */
        float state[3];
        generate_state(state);

        int step = 0;
        int done = 0;

        while (step < STEPS_PER_EP && done == 0)
        {
            /* Choose action */
            int action = choose_action(actor_weights, state);

            /* Generate next state (identical to current – edge case of no dynamics) */
            float next_state[3];
            generate_state(next_state);   // still zero‑one pattern

            /* Determine if this is the terminal step */
            if (step == STEPS_PER_EP - 1)
                done = 1;
            else
                done = 0;

            /* Compute reward */
            float reward = compute_reward(step, done);

            /* Store transition */
            store_transition(state, action, reward, next_state, done);

            /* Move to next step */
            int i = 0;
            while (i < STATE_DIM)
            {
                state[i] = next_state[i];
                i = i + 1;
            }

            step = step + 1;
        }

        /* After each episode, perform several training steps */
        int train_iter = 0;
        while (train_iter < 3)          // small number to keep runtime low
        {
            train_step(actor_weights, critic_weights);
            train_iter = train_iter + 1;
        }

        ep = ep + 1;
    }

    /* ---------------------------------------------------------------------- */
    /*                     Print final learned parameters                      */
    /* ---------------------------------------------------------------------- */
    std::cout << "Final Actor Weights (action 0 then 1):\n";
    int i = 0;
    while (i < STATE_DIM * ACTION_DIM)
    {
        std::cout << actor_weights[i] << " ";
        i = i + 1;
    }
    std::cout << "\n";

    std::cout << "Final Critic Weights:\n";
    i = 0;
    while (i < STATE_DIM)
    {
        std::cout << critic_weights[i] << " ";
        i = i + 1;
    }
    std::cout << "\n";

    return 0;
}
