#include <iostream>

/* LLM input variant 3: zeros-and-ones */
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
    // Zeros and ones polygon (quadrilateral)
    int points = 4;
    int xs[4] = {0, 1, 0, 1};
    int ys[4] = {0, 0, 1, 1};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
