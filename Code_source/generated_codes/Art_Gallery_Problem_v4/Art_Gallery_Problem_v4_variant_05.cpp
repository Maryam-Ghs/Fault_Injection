#include <iostream>

/* LLM input variant 5: duplicate-heavy */
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
    // Duplicate-heavy polygon (pentagon with repeated coordinates)
    int points = 5;
    int xs[5] = {2, 2, 2, 5, 5};
    int ys[5] = {3, 3, 3, 7, 7};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
