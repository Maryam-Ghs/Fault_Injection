#include <iostream>

/* LLM input variant 1: minimal-boundary */
class GallerySolver {
public:
    // Returns the minimum number of guards required for a simple polygon
    // using the classic floor(n/3) theorem.
    int computeGuards(int vertexNum, int xCoord[], int yCoord[]) {
        // temporary variables for clarity
        int vCount   = vertexNum;   // total vertices
        int divisor  = 3;           // divisor from the theorem
        int guards   = vCount / divisor; // integer division = floor(vCount/3)

        // The arrays are not used further, they just demonstrate stack allocation.
        // (In a real implementation you would verify polygon simplicity etc.)

        return guards;
    }
};

int main() {
    // Minimal valid polygon (triangle)
    int points = 3;
    int xs[3] = {0, 1, 0};
    int ys[3] = {0, 0, 1};

    // Instantiate the solver class
    GallerySolver solver;

    // Compute the guard count
    int neededGuards = solver.computeGuards(points, xs, ys);

    // Output the result
    std::cout << "Polygon vertices : " << points << std::endl;
    std::cout << "Minimum guards needed (floor(n/3)) : " << neededGuards << std::endl;

    return 0;
}
