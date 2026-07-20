#include <iostream>

/* LLM input variant 4: signed-extremes */
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
    // Signed extremes polygon (hexagon)
    int points = 6;
    int xs[6] = {-1000, -500, 0, 500, 1000, 250};
    int ys[6] = {1000, 0, -1000, 500, -250, 0};

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
