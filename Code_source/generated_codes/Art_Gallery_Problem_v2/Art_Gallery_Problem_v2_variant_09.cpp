#include <iostream>
#include <vector>

 /* LLM input variant 9: medium-deterministic-random */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // deterministic pseudo‑random pattern (15 vertices)
        const int N = 15;
        int xs[15] = {0,1,4,9,5,6,2,7,3,8,10,12,11,13,14};
        int ys[15] = {7,3,5,2,9,1,8,0,6,4,11,13,12,10,9};
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
