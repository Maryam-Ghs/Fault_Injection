#include <iostream>
#include <vector>

 /* LLM input variant 4: signed-extremes */
class GallerySolver {
public:
    struct Pt { int x; int y; };
    std::vector<Pt> verts;

    GallerySolver() {
        // mix of negative, zero, positive coordinates
        const int N = 6;
        int xs[6] = {-1000, -500, 0, 500, 1000, 0};
        int ys[6] = {-1000, 0, 1000, 0, -500, 500};
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
