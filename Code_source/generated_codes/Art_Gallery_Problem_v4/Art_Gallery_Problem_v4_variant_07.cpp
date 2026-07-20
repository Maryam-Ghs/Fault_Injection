#include <iostream>

/* LLM input variant 7: reverse-adversarial */
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
    // Reverse-ordered polygon (hexagon with descending coordinates)
    int points = 6;
    int xs[6] = {5, 4, 3, 2, 1, 0};
    int ys[6] = {5, 4, 3, 2, 1, 0};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
