#include <iostream>
#include <vector>

 /* LLM input variant 1: minimal-boundary */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // minimal polygon (triangle)
        const int N = 3;
        int xs[3] = {0, 1, 0};
        int ys[3] = {0, 0, 1};
        int idx = 0;
        while (idx < N) {
            verts.push_back({xs[idx], ys[idx]});
            idx = idx + 1;               // fused increment
        }
    }

    int guardsNeeded() {
        int n = (int)verts.size();      // fused cast
        return n / 3;                   // floor division, branch‑free
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
