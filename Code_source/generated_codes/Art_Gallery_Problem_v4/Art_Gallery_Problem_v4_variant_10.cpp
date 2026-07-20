#include <iostream>

/* LLM input variant 10: large-safe-stress */
class GallerySolver {
public:
    int computeGuards(int vertexNum, int /*xCoord*/[], int /*yCoord*/[]) {
        int vCount = vertexNum;
        int divisor = 3;
        int guards = vCount / divisor;
        return guards;
    }
};

int main() {
    // Large stress test polygon (300 vertices)
    const int points = 300;
    int xs[points];
    int ys[points];
    for (int i = 0; i < points; ++i) {
        xs[i] = i % 100;               // repeat pattern 0..99
        ys[i] = (points - i) % 100;    // reverse pattern 0..99
    }

    GallerySolver solver;
    int neededGuards = solver.computeGuards(points, xs, ys);

    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;
    return 0;
}
