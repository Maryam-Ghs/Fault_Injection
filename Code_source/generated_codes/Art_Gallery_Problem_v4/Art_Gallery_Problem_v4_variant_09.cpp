#include <iostream>

/* LLM input variant 9: medium-deterministic-random */
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
    // Medium deterministic‑random polygon (12 vertices)
    int points = 12;
    int xs[12] = {3, 5, 9, 2, 6, 13, 1, 12, 8, 4, 7, 11};
    int ys[12] = {7, 0, 14, 3, 11, 6, 9, 2, 5, 13, 1, 8};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
