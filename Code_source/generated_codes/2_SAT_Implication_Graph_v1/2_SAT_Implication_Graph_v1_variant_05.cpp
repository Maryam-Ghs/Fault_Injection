#include <bits/stdc++.h>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ------------------- parameter generation -------------------
    int var_cnt = 2;                 // number of variables
    int clause_cnt = 12;              // number of clauses
    // deterministic duplicate-heavy clause generation
    // using only literals 1 and -1 (variable x1) and 2 and -2 (variable x2)
    // many clauses will be identical or have the same literals
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
        int na = a > 0 ? -a : -a;
        int nb = b > 0 ? -b : -b;
        add_edge(node(na), node(b));
        add_edge(node(nb), node(a));
    };

    // ------------------- deterministic duplicate-heavy clause generation -------------------
    for (int i = 0; i < clause_cnt; ++i) {
        int a, b;
        // pattern creates many repeated clauses
        if (i % 4 == 0) a = 1;
        else if (i % 4 == 1) a = -1;
        else if (i % 4 == 2) a = 2;
        else a = -2;

        if (i % 3 == 0) b = 1;
        else if (i % 3 == 1) b = -1;
        else b = 2;

        add_clause(a, b);
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
