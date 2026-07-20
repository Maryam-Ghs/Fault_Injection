#include <iostream>
#include <vector>

 /* LLM input variant 2: small-diverse */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // small diverse polygon (5 vertices)
        const int N = 5;
        int xs[5] = {0, 2, 4, 3, 1};
        int ys[5] = {0, 0, 2, 4, 1};
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
