#include <iostream>
#include <vector>

 /* LLM input variant 6: ordered-structured */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // ordered symmetric polygon
        const int N = 7;
        int xs[7] = {-3, -2, -1, 0, 1, 2, 3};
        int ys[7] = {0, 1, 2, 3, 2, 1, 0};
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
