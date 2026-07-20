#include <iostream>
#include <vector>

 /* LLM input variant 8: sparse-skewed */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // sparse cluster at origin, few far points
        const int N = 10;
        int xs[10] = {0, 0, 0, 0, 0, 100, 101, 102, 103, 104};
        int ys[10] = {0, 0, 0, 0, 0,   0,   0,   0,   0,   0};
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
