#include <bits/stdc++.h>
#include <ctime>
#include <cstdlib>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // ------------------- parameter generation -------------------
    int var_cnt = 2;                     // number of variables (minimal, emphasize identity)
    int clause_cnt = 6;                  // number of clauses (small, deterministic)
    // srand not needed for deterministic input

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
    // Emphasize zeros and ones (using literals 1 and -1, same variable cases)
    std::vector<std::pair<int,int>> clauses = {
        { 1,  1},   // (x1 ∨ x1)
        { 1, -1},   // (x1 ∨ ¬x1)  (tautology)
        {-1, -1},   // (¬x1 ∨ ¬x1)
        { 2,  2},   // (x2 ∨ x2)
        { 2, -2},   // (x2 ∨ ¬x2)  (tautology)
        {-2, -2}    // (¬x2 ∨ ¬x2)
    };

    for (int i = 0; i < clause_cnt; ++i) {
        add_clause(clauses[i].first, clauses[i].second);
    }

    // ------------------- Kosaraju first pass -------------------
    std::vector<int> order;
    std::vector<char> used;
    used.assign(var_cnt * 2, 0);
    int v = 0;
    while (v < var_cnt * 2) {
        if (!used[v]) {
            // iterative DFS
            std::vector<std::pair<int,int>> st;
            st.push_back({v,0});
            while (!st.empty()) {
                int cur = st.back().first;
                int idx = st.back().second;
                if (!used[cur]) used[cur] = 1;
                if (idx < (int)g[cur].size()) {
                    int nxt = g[cur][idx];
                    st.back().second = idx + 1;
                    if (!used[nxt]) st.push_back({nxt,0});
                } else {
                    order.push_back(cur);
                    st.pop_back();
                }
            }
        }
        v = v + 1;
    }

    // ------------------- Kosaraju second pass -------------------
    std::vector<int> comp;
    comp.assign(var_cnt * 2, -1);
    int cur_comp = 0;
    int idx = (int)order.size() - 1;
    while (idx >= 0) {
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
                int j = 0;
                while (j < (int)rg[cur].size()) {
                    int nxt = rg[cur][j];
                    if (comp[nxt] == -1) stack.push_back(nxt);
                    j = j + 1;
                }
            }
            cur_comp = cur_comp + 1;
        }
        idx = idx - 1;
    }

    // ------------------- satisfiability check -------------------
    std::vector<int> assign;
    assign.assign(var_cnt, 0);
    int ok = 1;
    int k = 0;
    while (k < var_cnt) {
        if (comp[2*k] == comp[2*k+1]) {
            ok = 0;
            break;
        }
        assign[k] = comp[2*k] > comp[2*k+1];
        k = k + 1;
    }

    // ------------------- output -------------------
    if (ok) {
        printf("SAT\n");
        int p = 0;
        while (p < var_cnt) {
            printf("%d ", assign[p]);
            p = p + 1;
        }
        printf("\n");
    } else {
        printf("UNSAT\n");
    }
    return 0;
}
