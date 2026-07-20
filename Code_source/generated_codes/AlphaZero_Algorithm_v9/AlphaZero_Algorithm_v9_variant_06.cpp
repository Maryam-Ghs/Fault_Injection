#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

// ---------- deterministic float generator ----------
float deterministic_val(int idx, float scale = 0.001f) {
    // Generates values in range roughly [-0.5, 0.5] in a predictable pattern
    return ((float)idx * scale) - 0.5f;
}

// ---------- neural net weights (structured deterministic arrays) ----------
void init_weights(float w1[9][32], float b1[32],
                  float w2p[32][9], float b2p[9],
                  float w2v[32],   float &b2v) {
    int idx = 0;
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 32; ++j)
            w1[i][j] = deterministic_val(idx++);
    for (int i = 0; i < 32; ++i) b1[i] = deterministic_val(idx++);
    for (int i = 0; i < 32; ++i)
        for (int j = 0; j < 9; ++j)
            w2p[i][j] = deterministic_val(idx++);
    for (int i = 0; i < 9; ++i) b2p[i] = deterministic_val(idx++);
    for (int i = 0; i < 32; ++i) w2v[i] = deterministic_val(idx++);
    b2v = deterministic_val(idx++);
}

// ---------- forward pass: returns policy (9) and value (1) ----------
void evaluate(const int board[9],
              const float w1[9][32], const float b1[32],
              const float w2p[32][9], const float b2p[9],
              const float w2v[32],   const float b2v,
              float policy[9], float &value) {
    float hidden[32];
    for (int i = 0; i < 32; ++i) {
        float acc = b1[i];
        for (int j = 0; j < 9; ++j)
            acc += w1[j][i] * (float)board[j];
        hidden[i] = 1.0f / (1.0f + expf(-acc));
    }
    for (int i = 0; i < 9; ++i) {
        float acc = b2p[i];
        for (int j = 0; j < 32; ++j)
            acc += w2p[j][i] * hidden[j];
        policy[i] = 1.0f / (1.0f + expf(-acc));
    }
    float v = b2v;
    for (int i = 0; i < 32; ++i) v += w2v[i] * hidden[i];
    value = tanhf(v);
}

// ---------- MCTS node ----------
struct Node {
    int move;
    int visits;
    float total;
    float prior;
    Node *child[9];
    Node() : move(-1), visits(0), total(0.0f), prior(0.0f) {
        for (int i = 0; i < 9; ++i) child[i] = nullptr;
    }
};

// ---------- selection (UCB) ----------
Node* select(Node *root, float cpuct) {
    Node *cur = root;
    while (true) {
        int best = -1; float bestScore = -1e30f;
        for (int i = 0; i < 9; ++i) if (cur->child[i]) {
            Node *c = cur->child[i];
            float q = c->visits ? c->total / (float)c->visits : 0.0f;
            float u = cpuct * c->prior * sqrtf((float)cur->visits) / (1.0f + (float)c->visits);
            float score = q + u;
            bestScore = (score > bestScore) * score + (score <= bestScore) * bestScore;
            best = (score > bestScore) * i + (score <= bestScore) * best;
        }
        if (best == -1) break;
        cur = cur->child[best];
    }
    return cur;
}

// ---------- expansion ----------
void expand(Node *node, const float policy[9]) {
    for (int i = 0; i < 9; ++i) if (policy[i] > 0.0f) {
        node->child[i] = new Node();
        node->child[i]->move = i;
        node->child[i]->prior = policy[i];
    }
}

// ---------- back‑propagation ----------
void backup(Node *leaf, float value) {
    Node *cur = leaf;
    while (cur) {
        cur->visits += 1;
        cur->total += value;
        value = -value;
        cur = (cur->move == -1) ? nullptr : cur;
    }
}

// ---------- single MCTS simulation ----------
void simulate(Node *root, const int board[9],
              const float w1[9][32], const float b1[32],
              const float w2p[32][9], const float b2p[9],
              const float w2v[32],   const float b2v,
              float cpuct) {
    Node *leaf = select(root, cpuct);
    int tmpBoard[9];
    for (int i = 0; i < 9; ++i) tmpBoard[i] = board[i];
    if (leaf->move != -1) tmpBoard[leaf->move] = 1;
    float policy[9]; float value;
    evaluate(tmpBoard, w1, b1, w2p, b2p, w2v, b2v, policy, value);
    expand(leaf, policy);
    backup(leaf, value);
}

// ---------- run MCTS ----------
void run_mcts(const int board[9],
              const float w1[9][32], const float b1[32],
              const float w2p[32][9], const float b2p[9],
              const float w2v[32],   const float b2v,
              int simulations, float cpuct,
              float outPolicy[9]) {
    Node root;
    float initPol[9]; float initVal;
    evaluate(board, w1, b1, w2p, b2p, w2v, b2v, initPol, initVal);
    expand(&root, initPol);
    for (int i = 0; i < simulations; ++i)
        simulate(&root, board, w1, b1, w2p, b2p, w2v, b2v, cpuct);
    float sum = 0.0f;
    for (int i = 0; i < 9; ++i) {
        outPolicy[i] = root.child[i] ? (float)root.child[i]->visits : 0.0f;
        sum += outPolicy[i];
    }
    for (int i = 0; i < 9; ++i) outPolicy[i] = sum ? outPolicy[i] / sum : 0.0f;
}

// ---------- main ----------
int main() {
    // Structured board: three -1s, three 0s, three 1s in order
    int board[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };

    // Initialise deterministic weight matrices
    float w1[9][32], b1[32];
    float w2p[32][9], b2p[9];
    float w2v[32], b2v;
    init_weights(w1, b1, w2p, b2p, w2v, b2v);

    // Run MCTS
    const int sims = 500;
    const float cp = 1.5f;
    float finalPol[9];
    run_mcts(board, w1, b1, w2p, b2p, w2v, b2v, sims, cp, finalPol);

    // Print board and resulting policy
    printf("Structured board ( -1 = O, 0 = ., 1 = X ):\n");
    for (int i = 0; i < 9; ++i) {
        printf("%2d ", board[i]);
        if ((i + 1) % 3 == 0) printf("\n");
    }
    printf("\nPolicy (move probabilities):\n");
    for (int i = 0; i < 9; ++i)
        printf("%6.3f ", finalPol[i]);
    printf("\n");
    return 0;
}
