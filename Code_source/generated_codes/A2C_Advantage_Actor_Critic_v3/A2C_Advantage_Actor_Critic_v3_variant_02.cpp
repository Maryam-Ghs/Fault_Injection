#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 2: small-diverse */

class A2C {
public:
    int sdim;
    int adim;
    std::vector<float> wp;   // policy weights: sdim * adim
    std::vector<float> wv;   // value weights: sdim

    A2C(int s, int a) : sdim(s), adim(a),
        wp(s * a, 0.05f), wv(s, 0.05f) {}

    // compute linear output for policy (logits)
    void logits(const std::vector<float>& st, std::vector<float>& out) {
        int i = 0;
        while (i < adim) {
            float acc = 0.0f;
            int j = 0;
            while (j < sdim) {
                acc = acc + st[j] * wp[j * adim + i];
                j = j + 1;
            }
            out[i] = acc;
            i = i + 1;
        }
    }

    // softmax producing probabilities
    void softmax(const std::vector<float>& lg, std::vector<float>& pr) {
        int i = 0;
        float maxv = lg[0];
        while (i < adim) {
            maxv = (lg[i] > maxv) ? lg[i] : maxv;
            i = i + 1;
        }
        i = 0;
        float sum = 0.0f;
        while (i < adim) {
            float e = std::exp(lg[i] - maxv);
            pr[i] = e;
            sum = sum + e;
            i = i + 1;
        }
        i = 0;
        while (i < adim) {
            pr[i] = pr[i] / sum;
            i = i + 1;
        }
    }

    // value estimation
    float value(const std::vector<float>& st) {
        float val = 0.0f;
        int i = 0;
        while (i < sdim) {
            val = val + st[i] * wv[i];
            i = i + 1;
        }
        return val;
    }

    // single learning step
    void learn(const std::vector<float>& cur,
               int act,
               float rew,
               const std::vector<float>& nxt,
               int fin) {
        // forward pass current state
        std::vector<float> cur_log(adim, 0.0f);
        logits(cur, cur_log);
        std::vector<float> cur_prob(adim, 0.0f);
        softmax(cur_log, cur_prob);
        float cur_val = value(cur);

        // forward pass next state
        float nxt_val = value(nxt);
        float term = 1.0f - float(fin);
        float td_target = rew + term * nxt_val;
        float adv = td_target - cur_val;

        // policy gradient update
        int i = 0;
        while (i < adim) {
            float grad = adv * ((i == act ? 1.0f : 0.0f) - cur_prob[i]);
            int j = 0;
            while (j < sdim) {
                wp[j * adim + i] = wp[j * adim + i] + 0.1f * grad * cur[j];
                j = j + 1;
            }
            i = i + 1;
        }

        // value gradient update
        i = 0;
        while (i < sdim) {
            wv[i] = wv[i] + 0.1f * adv * cur[i];
            i = i + 1;
        }
    }

    // get policy for a state
    void policy(const std::vector<float>& st, std::vector<float>& out) {
        std::vector<float> lg(adim, 0.0f);
        logits(st, lg);
        softmax(lg, out);
    }
};

int main() {
    // deterministic test vectors
    int state_dim = 3;
    int action_dim = 3;
    A2C agent(state_dim, action_dim);

    std::vector<float> s0 = {0.5f, -0.2f, 0.1f};
    std::vector<float> s1 = {1.0f, 0.0f, -0.5f};
    std::vector<float> s2 = {0.0f, 0.3f, 0.8f};

    std::vector<std::vector<float>> states = {s0, s1, s2};
    std::vector<int> actions = {2, 0, 1};
    std::vector<float> rewards = {0.5f, -0.2f, 1.0f};

    int step = 0;
    while (step < 3) {
        int fin = (step == 2) ? 1 : 0;
        std::vector<float> nxt = (step < 2) ? states[step + 1] : states[2];
        agent.learn(states[step], actions[step], rewards[step], nxt, fin);
        step = step + 1;
    }

    // print final policy for each state
    int idx = 0;
    while (idx < 3) {
        std::vector<float> prob(action_dim, 0.0f);
        agent.policy(states[idx], prob);
        std::cout << "State " << idx << " policy: ";
        int a = 0;
        while (a < action_dim) {
            std::cout << prob[a] << (a + 1 == action_dim ? "" : ", ");
            a = a + 1;
        }
        std::cout << std::endl;
        idx = idx + 1;
    }
    return 0;
}
