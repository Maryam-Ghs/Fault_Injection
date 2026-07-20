#include <iostream>

/* LLM input variant 6: ordered-structured */
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
    // Ordered-structured polygon (hexagon with monotonic coordinates)
    int points = 6;
    int xs[6] = {0, 1, 2, 3, 4, 5};
    int ys[6] = {0, 1, 2, 3, 4, 5};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
