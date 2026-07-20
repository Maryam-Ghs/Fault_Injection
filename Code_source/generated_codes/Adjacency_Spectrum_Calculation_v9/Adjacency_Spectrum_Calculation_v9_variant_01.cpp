/* LLM input variant 1: minimal-boundary */

// Version #9 – Adjacency Spectrum Calculation (verbose, step‑by‑step, float only)

#include <iostream>
#include <vector>
#include <cmath>

// -------------------------------------------------------------------
// Helper: fill an adjacency matrix with a deterministic undirected graph
// -------------------------------------------------------------------
void buildAdjacency(std::vector<std::vector<float>>& graph, int nodeCount)
{
    // Initialise all entries to zero
    int i = 0;
    while (i < nodeCount)
    {
        int j = 0;
        while (j < nodeCount)
        {
            graph[i][j] = 0.0f;
            ++j;
        }
        ++i;
    }

    // Deterministic edges (minimal 2‑node example)
    if (nodeCount >= 2)
    {
        graph[0][1] = 1.0f; graph[1][0] = 1.0f;
    }
}

// -------------------------------------------------------------------
// Helper: print a one‑dimensional vector (eigenvalues)
// -------------------------------------------------------------------
void displayValues(const std::vector<float>& values)
{
    int idx = 0;
    while (idx < static_cast<int>(values.size()))
    {
        std::cout << "Eigenvalue[" << idx << "] = " << values[idx] << '\n';
        ++idx;
    }
}

// -------------------------------------------------------------------
// Core: Jacobi rotation method for symmetric matrices (float version)
// -------------------------------------------------------------------
void jacobiEigenvalues(std::vector<std::vector<float>>& a,
                       std::vector<float>& eigenVals,
                       int iterationLimit,
                       float tolerance)
{
    int n = static_cast<int>(a.size());

    // Initialise eigenVals with diagonal elements (will be refined)
    int p = 0;
    while (p < n)
    {
        eigenVals[p] = a[p][p];
        ++p;
    }

    int iteration = 0;
    while (iteration < iterationLimit)
    {
        // -------------------------------------------------------------
        // 1. Find the largest off‑diagonal absolute element
        // -------------------------------------------------------------
        float biggest = 0.0f;
        int rowMax = 0;
        int colMax = 1; // start with first off‑diagonal element

        int r = 0;
        while (r < n - 1)
        {
            int c = r + 1;
            while (c < n)
            {
                float candidate = std::fabs(a[r][c]);
                if (candidate > biggest)
                {
                    biggest = candidate;
                    rowMax = r;
                    colMax = c;
                }
                ++c;
            }
            ++r;
        }

        // If the biggest off‑diagonal element is already below tolerance,
        // we consider the matrix diagonalised.
        if (biggest < tolerance) break;

        // -------------------------------------------------------------
        // 2. Compute the Jacobi rotation parameters for (rowMax, colMax)
        // -------------------------------------------------------------
        float angle = 0.0f;
        float diff   = a[colMax][colMax] - a[rowMax][rowMax];
        float sum    = a[rowMax][colMax] * 2.0f;

        // Avoid division by zero – handle the special case explicitly
        if (std::fabs(diff) < 1e-6f)
        {
            angle = (sum >= 0.0f) ? 0.785398163f : -0.785398163f; // ±π/4
        }
        else
        {
            float tan2Theta = sum / diff;
            angle = 0.5f * std::atan(tan2Theta);
        }

        float sinTheta = std::sin(angle);
        float cosTheta = std::cos(angle);
        float sin2Theta = sinTheta * sinTheta;
        float cos2Theta = cosTheta * cosTheta;
        float sinCos = sinTheta * cosTheta;

        // -------------------------------------------------------------
        // 3. Apply rotation – update matrix entries
        // -------------------------------------------------------------
        // Update the diagonal elements first (they depend only on the
        // pivot positions)
        float a_pp = a[rowMax][rowMax];
        float a_qq = a[colMax][colMax];
        float a_pq = a[rowMax][colMax];

        float new_pp = cos2Theta * a_pp - 2.0f * sinCos * a_pq + sin2Theta * a_qq;
        float new_qq = sin2Theta * a_pp + 2.0f * sinCos * a_pq + cos2Theta * a_qq;

        a[rowMax][rowMax] = new_pp;
        a[colMax][colMax] = new_qq;
        a[rowMax][colMax] = 0.0f;
        a[colMax][rowMax] = 0.0f; // maintain symmetry

        // Update the remaining rows/columns
        int k = 0;
        while (k < n)
        {
            if (k != rowMax && k != colMax)
            {
                float a_pk = a[rowMax][k];
                float a_qk = a[colMax][k];

                float new_pk =  cosTheta * a_pk - sinTheta * a_qk;
                float new_qk =  sinTheta * a_pk + cosTheta * a_qk;

                a[rowMax][k] = new_pk;
                a[k][rowMax] = new_pk; // symmetry
                a[colMax][k] = new_qk;
                a[k][colMax] = new_qk; // symmetry
            }
            ++k;
        }

        ++iteration;
    }

    // After convergence, copy the diagonal into eigenVals
    int idx = 0;
    while (idx < n)
    {
        eigenVals[idx] = a[idx][idx];
        ++idx;
    }
}

// -------------------------------------------------------------------
// Entry point – generate data, compute spectrum, display
// -------------------------------------------------------------------
int main()
{
    // ----------------------------------------------------------------
    // Step 1: Define the size of the graph (deterministic)
    // ----------------------------------------------------------------
    int vertexCount = 2; // minimal non‑trivial graph

    // ----------------------------------------------------------------
    // Step 2: Allocate a square matrix using std::vector
    // ----------------------------------------------------------------
    std::vector<std::vector<float>> adjacency;
    int i = 0;
    while (i < vertexCount)
    {
        std::vector<float> row(vertexCount, 0.0f);
        adjacency.push_back(row);
        ++i;
    }

    // ----------------------------------------------------------------
    // Step 3: Populate the matrix with a known undirected graph
    // ----------------------------------------------------------------
    buildAdjacency(adjacency, vertexCount);

    // ----------------------------------------------------------------
    // Step 4: Prepare storage for eigenvalues
    // ----------------------------------------------------------------
    std::vector<float> spectrum(vertexCount, 0.0f);

    // ----------------------------------------------------------------
    // Step 5: Run the Jacobi eigenvalue routine (verbose parameters)
    // ----------------------------------------------------------------
    int maxIterations = 100;          // safety bound
    float eps = 1e-5f;                // convergence tolerance
    jacobiEigenvalues(adjacency, spectrum, maxIterations, eps);

    // ----------------------------------------------------------------
    // Step 6: Output the resulting spectrum
    // ----------------------------------------------------------------
    std::cout << "Adjacency spectrum (float precision):\n";
    displayValues(spectrum);

    return 0;
}
