#include <bits/stdc++.h>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ------------------- parameter generation -------------------
    int var_cnt = 800;                 // number of variables (adjusted)
    int clause_cnt = 4000;             // number of clauses (adjusted)

    // deterministic pseudo-random generator (LCG)
    uint64_t rng_state = 123456789ULL;
    auto next_rand = [&]() -> uint32_t {
        rng_state = rng_state * 6364136223846793005ULL + 1;
        return static_cast<uint32_t>(rng_state >> 32);
    };

    // ------------------- data structures -------------------
    std::vector< std::vector<int> > g;          // implication graph
    std::vector< std::vector<int> > rg;         // reversed graph
    g.resize(var_cnt * 2);
    rg.resize(var_cnt * 2);

    // ------------------- helper lambdas -------------------
    auto node = [&](int lit) -> int {
        // lit > 0  => x_i is true, lit < 0 => x_i is false
        int v = lit > 0 ? lit - 1 : -lit - 1;
        return lit > 0 ? v * 2 : v * 2 + 1;
    };

    auto add_edge = [&](int from, int to) {
        g[from].push_back(to);
        rg[to].push_back(from);
    };

    auto add_clause = [&](int a, int b) {
        // (a ∨ b)  =>  (¬a → b)  and  (¬b → a)
        int na = -a;
        int nb = -b;
        add_edge(node(na), node(b));
        add_edge(node(nb), node(a));
    };

    // ------------------- deterministic clause generation -------------------
    for (int i = 0; i < clause_cnt; ++i) {
        int x = (next_rand() % var_cnt) + 1;
        int y = (next_rand() % var_cnt) + 1;
        int sign_x = (next_rand() % 2) ? 1 : -1;   // 1 or -1
        int sign_y = (next_rand() % 2) ? 1 : -1;
        add_clause(sign_x * x, sign_y * y);
    }

    // ------------------- Kosaraju first pass -------------------
    std::vector<int> order;
    std::vector<char> used(var_cnt * 2, 0);
    for (int v = 0; v < var_cnt * 2; ++v) {
        if (!used[v]) {
            // iterative DFS
            std::vector<std::pair<int,int>> st;
            st.emplace_back(v, 0);
            while (!st.empty()) {
                int cur = st.back().first;
                int idx = st.back().second;
                if (!used[cur]) used[cur] = 1;
                if (idx < (int)g[cur].size()) {
                    int nxt = g[cur][idx];
                    st.back().second = idx + 1;
                    if (!used[nxt]) st.emplace_back(nxt, 0);
                } else {
                    order.push_back(cur);
                    st.pop_back();
                }
            }
        }
    }

    // ------------------- Kosaraju second pass -------------------
    std::vector<int> comp(var_cnt * 2, -1);
    int cur_comp = 0;
    for (int idx = (int)order.size() - 1; idx >= 0; --idx) {
        int start = order[idx];
        if (comp[start] == -1) {
            // iterative DFS on reversed graph
            std::vector<int> stack;
            stack.push_back(start);
            while (!stack.empty()) {
                int cur = stack.back();
                stack.pop_back();
                if (comp[cur] != -1) continue;
                comp[cur] = cur_comp;
                for (int nxt : rg[cur]) {
                    if (comp[nxt] == -1) stack.push_back(nxt);
                }
            }
            ++cur_comp;
        }
    }

    // ------------------- satisfiability check -------------------
    std::vector<int> assign(var_cnt, 0);
    bool ok = true;
    for (int k = 0; k < var_cnt; ++k) {
        if (comp[2*k] == comp[2*k+1]) {
            ok = false;
            break;
        }
        assign[k] = comp[2*k] > comp[2*k+1];
    }

    // ------------------- output -------------------
    if (ok) {
        printf("SAT\n");
        for (int p = 0; p < var_cnt; ++p) {
            printf("%d ", assign[p]);
        }
        printf("\n");
    } else {
        printf("UNSAT\n");
    }
    return 0;
}
