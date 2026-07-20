#include <iostream>

/* LLM input variant 8: sparse-skewed */
class GallerySolver {
public:
    int computeGuards(int vertexNum, int xCoord[], int yCoord[]) {
        int vCount = vertexNum;
        int divisor = 3;
        int guards = vCount / divisor;
        return guards;
    }
};

int main() {
    // Sparse-skewed polygon (mostly zeros, few non‑zero points)
    int points = 7;
    int xs[7] = {0, 0, 0, 0, 10, 0, 0};
    int ys[7] = {0, 0, 5, 0, 0, 0, 0};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
