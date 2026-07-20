#include <iostream>

/* LLM input variant 2: small-diverse */
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
    // Small diverse polygon (pentagon)
    int points = 5;
    int xs[5] = {0, 2, 4, 3, 1};
    int ys[5] = {0, 1, 0, 2, 3};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
