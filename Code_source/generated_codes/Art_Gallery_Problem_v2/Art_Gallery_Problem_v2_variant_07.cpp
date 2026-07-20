#include <iostream>
#include <vector>

 /* LLM input variant 7: reverse-adversarial */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // reverse-ordered pattern
        const int N = 9;
        int xs[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        int ys[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
        int idx = 0;
        while (idx < N) {
            verts.push_back({xs[idx], ys[idx]});
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
