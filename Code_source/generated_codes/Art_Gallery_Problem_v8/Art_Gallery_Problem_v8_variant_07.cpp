#include <bits/stdc++.h>
using namespace std;

//------------------------------------------------------------
// Helper structures and functions
//------------------------------------------------------------
struct Pt {
    int x;
    int y;
};

int cross_product(const Pt& a, const Pt& b, const Pt& c) {
    // (b - a) × (c - a)
    int dx1 = b.x - a.x;
    int dy1 = b.y - a.y;
    int dx2 = c.x - a.x;
    int dy2 = c.y - a.y;

    int mult1 = dx1 * dy2;
    int mult2 = dy1 * dx2;

    int result = mult1 - mult2;
    return result;
}

/* LLM input variant 7: reverse-adversarial */
//------------------------------------------------------------
// Class handling the whole problem
//------------------------------------------------------------
class GuardPlanner {
    vector<Pt> pts;          // all generated points
    vector<Pt> hull;         // convex hull of points
    int vertex_cnt;          // number of vertices in hull
    int guard_cnt;           // minimum guards needed

public:
    GuardPlanner(int sz) {
        vertex_cnt = 0;
        guard_cnt  = 0;
        generate_random_points(sz);
        build_convex_hull();
        compute_guards();
    }

    void generate_random_points(int sz) {
        srand(12345);
        pts.clear();
        int generated = 0;
        while (generated < sz) {
            Pt p;
            // Produce points that will be reverse‑ordered after sorting
            p.x = 100 - (rand() % 20);   // values 80..99
            p.y = 100 - (rand() % 20);
            pts.push_back(p);
            generated = generated + 1;
        }
    }

    void build_convex_hull() {
        // Sort points lexicographically
        sort(pts.begin(), pts.end(),
            [](const Pt& a, const Pt& b) {
                if (a.x != b.x) return a.x < b.x;
                return a.y < b.y;
            });

        vector<Pt> lower;
        int i = 0;
        while (i < (int)pts.size()) {
            while (lower.size() >= 2) {
                Pt a = lower[lower.size() - 2];
                Pt b = lower[lower.size() - 1];
                Pt c = pts[i];
                int cp = cross_product(a, b, c);
                if (cp <= 0) {
                    lower.pop_back();
                } else {
                    break;
                }
            }
            lower.push_back(pts[i]);
            i = i + 1;
        }

        vector<Pt> upper;
        i = (int)pts.size() - 1;
        while (i >= 0) {
            while (upper.size() >= 2) {
                Pt a = upper[upper.size() - 2];
                Pt b = upper[upper.size() - 1];
                Pt c = pts[i];
                int cp = cross_product(a, b, c);
                if (cp <= 0) {
                    upper.pop_back();
                } else {
                    break;
                }
            }
            upper.push_back(pts[i]);
            i = i - 1;
        }

        // Concatenate lower and upper to obtain full hull
        hull.clear();
        int idx = 0;
        while (idx < (int)lower.size() - 1) {
            hull.push_back(lower[idx]);
            idx = idx + 1;
        }
        idx = 0;
        while (idx < (int)upper.size() - 1) {
            hull.push_back(upper[idx]);
            idx = idx + 1;
        }

        vertex_cnt = hull.size();
    }

    void compute_guards() {
        // The Art Gallery theorem: ⌊n/3⌋ guards
        int division = vertex_cnt / 3;
        int remainder = vertex_cnt % 3;
        int temp = division;
        if (remainder < 0) {
            temp = temp - 1;
        }
        guard_cnt = temp;
    }

    void report() {
        cout << "Generated polygon vertices (convex Hull):" << endl;
        int k = 0;
        while (k < (int)hull.size()) {
            cout << "(" << hull[k].x << ", " << hull[k].y << ")" << endl;
            k = k + 1;
        }
        cout << "Number of vertices = " << vertex_cnt << endl;
        cout << "Minimum number of guards  = " << guard_cnt << endl;
    }
};

//------------------------------------------------------------
// Entry point
//------------------------------------------------------------
int main() {
    // Choose a size for the random point set
    int point_set_size = 9;   // reverse‑adversarial size

    GuardPlanner planner(point_set_size);
    planner.report();

    return 0;
}
