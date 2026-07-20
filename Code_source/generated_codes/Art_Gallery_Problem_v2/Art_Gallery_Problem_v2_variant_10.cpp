#include <iostream>
#include <vector>

 /* LLM input variant 10: large-safe-stress */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // generate 1000 vertices with simple deterministic pattern
        const int N = 1000;
        int idx = 0;
        while (idx < N) {
            int x = idx % 100;              // cycles 0‑99
            int y = (idx * 2) % 100;        // cycles 0‑99, different phase
            verts.push_back({x, y});
            idx = idx + 1;
        }
    }

    int guardsNeeded() {
        int n = (int)verts.size();
        return n / 3;
    }

    void showResult() {
        int g = guardsNeeded();
        std::cout << "Vertex count: " << verts.size() << '\n';
        std::cout << "Minimum guards required: " << g << '\n';
    }
};

int main() {
    GallerySolver solver;
    solver.showResult();
    return 0;
}
