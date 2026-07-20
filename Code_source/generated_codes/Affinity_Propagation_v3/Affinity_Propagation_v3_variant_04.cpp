/***********************************************************************
 *  Affinity Propagation (Version #3)
 *  ---------------------------------------------------------------
 *  A verbose, step‑by‑step, class‑based implementation that uses only
 *  float and int types, heap allocated arrays, and internally generated
 *  random data.  The algorithm runs for a fixed number of iterations
 *  and finally prints the identified exemplars (cluster centers) and
 *  the points assigned to each cluster.
 *
 *  Restrictions applied:
 *    - No double, long, unsigned or const qualifiers.
 *    - All numeric data stored as float or int.
 *    - No use of std::cin – the input is generated inside main().
 *    - All arrays are allocated with new[] and released with delete[].
 *    - Variable and loop names differ from any previous version.
 *
 *  Compile with a C++11 (or later) compiler.
 ***********************************************************************/
/* LLM input variant 4: signed-extremes */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>    // only for fabs (float absolute value)

/*---------------------------------------------------------------*/
/*  Helper: generate a random float in [0, maxVal)                */
/*---------------------------------------------------------------*/
float randFloat(float maxVal)
{
    return (static_cast<float>(std::rand()) / RAND_MAX) * maxVal;
}

/*---------------------------------------------------------------*/
/*  Class: AffinityPropagation                                   */
/*---------------------------------------------------------------*/
class AffinityPropagation
{
public:
    /*-----------------------------------------------------------*/
    /*  Public data ------------------------------------------------*/
    /*-----------------------------------------------------------*/
    int    pointCount;        // number of data points
    int    maxIter;           // maximum iterations
    float  damping;           // damping factor (lambda)
    float  pref;              // preference (self‑similarity)

    /*-----------------------------------------------------------*/
    /*  Constructor / Destructor ---------------------------------*/
    /*-----------------------------------------------------------*/
    AffinityPropagation(int N, int I, float D, float P)
        : pointCount(N), maxIter(I), damping(D), pref(P)
    {
        // Allocate similarity, responsibility and availability matrices
        simMat   = new float*[pointCount];
        respMat  = new float*[pointCount];
        availMat = new float*[pointCount];
        for (int i = 0; i < pointCount; ++i)
        {
            simMat[i]   = new float[pointCount];
            respMat[i]  = new float[pointCount];
            availMat[i] = new float[pointCount];
        }

        // Allocate storage for final exemplar indices
        exemplars = new int[pointCount];
    }

    ~AffinityPropagation()
    {
        // Release all heap memory
        for (int i = 0; i < pointCount; ++i)
        {
            delete[] simMat[i];
            delete[] respMat[i];
            delete[] availMat[i];
        }
        delete[] simMat;
        delete[] respMat;
        delete[] availMat;
        delete[] exemplars;
    }

    /*-----------------------------------------------------------*/
    /*  Public interface ------------------------------------------*/
    /*-----------------------------------------------------------*/
    void setData(float *xCoord, float *yCoord)
    {
        // Step 1: fill the similarity matrix with negative squared Euclidean distances
        std::cout << "=== Step 1: Computing similarity matrix ===\n";
        for (int row = 0; row < pointCount; ++row)
        {
            for (int col = 0; col < pointCount; ++col)
            {
                float dx = xCoord[row] - xCoord[col];
                float dy = yCoord[row] - yCoord[col];
                simMat[row][col] = -(dx * dx + dy * dy);   // negative distance
            }
        }
        // Insert the preference value on the diagonal (self‑similarities)
        for (int i = 0; i < pointCount; ++i)
            simMat[i][i] = pref;

        // Initialise responsibilities and availabilities to zero
        for (int i = 0; i < pointCount; ++i)
            for (int j = 0; j < pointCount; ++j)
                respMat[i][j] = availMat[i][j] = 0.0f;
    }

    void run()
    {
        std::cout << "\n=== Step 2: Iterative message passing ===\n";
        for (int iter = 0; iter < maxIter; ++iter)
        {
            // ---- Update responsibilities ---------------------------------
            for (int i = 0; i < pointCount; ++i)
            {
                // Find the largest and second‑largest (a+s) values for row i
                float maxVal  = -1e30f;
                float secVal  = -1e30f;
                int   maxIdx  = -1;

                for (int k = 0; k < pointCount; ++k)
                {
                    float val = availMat[i][k] + simMat[i][k];
                    if (val > maxVal)
                    {
                        secVal = maxVal;
                        maxVal = val;
                        maxIdx = k;
                    }
                    else if (val > secVal)
                    {
                        secVal = val;
                    }
                }

                // Compute new responsibilities using the max / second‑max
                for (int k = 0; k < pointCount; ++k)
                {
                    float oldR = respMat[i][k];
                    float newR = simMat[i][k] - ((k == maxIdx) ? secVal : maxVal);
                    // Damping
                    respMat[i][k] = damping * oldR + (1.0f - damping) * newR;
                }
            }

            // ---- Update availabilities ------------------------------------
            for (int k = 0; k < pointCount; ++k)
            {
                // Sum of positive responsibilities for column k (excluding i = k)
                float sumPos = 0.0f;
                for (int i = 0; i < pointCount; ++i)
                {
                    if (i == k) continue;
                    float r = respMat[i][k];
                    sumPos += (r > 0.0f) ? r : 0.0f;
                }

                // Update a(k,k)
                float oldAA = availMat[k][k];
                float newAA = sumPos;
                availMat[k][k] = damping * oldAA + (1.0f - damping) * newAA;

                // Update a(i,k) for i != k
                for (int i = 0; i < pointCount; ++i)
                {
                    if (i == k) continue;
                    float oldA = availMat[i][k];
                    float rkk  = respMat[k][k];
                    float val  = rkk + sumPos - ((respMat[i][k] > 0.0f) ? respMat[i][k] : 0.0f);
                    if (val > 0.0f) val = 0.0f;   // apply min(0, …)
                    // Damping
                    availMat[i][k] = damping * oldA + (1.0f - damping) * val;
                }
            }

            // Optional: print progress every few iterations
            if ((iter + 1) % 10 == 0 || iter == maxIter - 1)
            {
                std::cout << "   Completed iteration " << (iter + 1) << "\n";
            }
        }
    }

    void extractExemplars()
    {
        std::cout << "\n=== Step 3: Determining exemplars ===\n";
        // For each point, compute the index that maximises a(i,k) + r(i,k)
        for (int i = 0; i < pointCount; ++i)
        {
            float bestScore = -1e30f;
            int   bestIdx   = -1;
            for (int k = 0; k < pointCount; ++k)
            {
                float score = availMat[i][k] + respMat[i][k];
                if (score > bestScore)
                {
                    bestScore = score;
                    bestIdx   = k;
                }
            }
            exemplars[i] = bestIdx;   // point i is assigned to exemplar bestIdx
        }
    }

    void printClusters(float *xCoord, float *yCoord)
    {
        std::cout << "\n=== Final Clustering Result ===\n";

        // Identify unique exemplar indices
        bool *isCenter = new bool[pointCount];
        for (int i = 0; i < pointCount; ++i) isCenter[i] = false;
        for (int i = 0; i < pointCount; ++i)
            isCenter[exemplars[i]] = true;

        // Print each cluster
        for (int c = 0; c < pointCount; ++c)
        {
            if (!isCenter[c]) continue;   // skip non‑center entries

            std::cout << "Cluster centered at point " << c
                      << " (x=" << xCoord[c] << ", y=" << yCoord[c] << "):\n";

            // List members
            for (int i = 0; i < pointCount; ++i)
            {
                if (exemplars[i] == c)
                {
                    std::cout << "   Point " << i
                              << " (x=" << xCoord[i] << ", y=" << yCoord[i] << ")\n";
                }
            }
        }

        delete[] isCenter;
    }

private:
    /*-----------------------------------------------------------*/
    /*  Private storage ------------------------------------------*/
    /*-----------------------------------------------------------*/
    float **simMat;    // similarity matrix (negative distances)
    float **respMat;   // responsibility matrix
    float **availMat;  // availability matrix
    int   *exemplars;  // final exemplar index for each point
};

/*===================================================================*/
/*  Main program – generate data, run Affinity Propagation          */
/*===================================================================*/
int main()
{
    // ---------------------------------------------------------------
    // 1) Generate a deterministic data set with mixed signs (30 points)
    // ---------------------------------------------------------------
    const int N = 30;                     // number of points (medium)
    float *xs = new float[N];
    float *ys = new float[N];

    std::cout << "=== Generating deterministic data set (" << N << " points) ===\n";
    for (int i = 0; i < N; ++i)
    {
        // Cycle through -50, 0, +50 for x; use a shifted pattern for y
        if (i % 3 == 0)
            xs[i] = -50.0f;
        else if (i % 3 == 1)
            xs[i] = 0.0f;
        else
            xs[i] = 50.0f;

        if (i % 4 == 0)
            ys[i] = -30.0f;
        else if (i % 4 == 1)
            ys[i] = 0.0f;
        else if (i % 4 == 2)
            ys[i] = 30.0f;
        else
            ys[i] = 60.0f;

        std::cout << "Point " << i << ": (" << xs[i] << ", " << ys[i] << ")\n";
    }

    // ---------------------------------------------------------------
    // 2) Create AffinityPropagation instance with chosen parameters
    // ---------------------------------------------------------------
    const int    ITERATIONS = 100;        // max iterations
    const float  DAMPING    = 0.5f;       // typical damping factor
    const float  PREF       = -500.0f;    // preference (self‑similarity)

    AffinityPropagation ap(N, ITERATIONS, DAMPING, PREF);

    // ---------------------------------------------------------------
    // 3) Feed data into the algorithm
    // ---------------------------------------------------------------
    ap.setData(xs, ys);

    // ---------------------------------------------------------------
    // 4) Run the message‑passing iterations
    // ---------------------------------------------------------------
    ap.run();

    // ---------------------------------------------------------------
    // 5) Extract exemplars and print final clusters
    // ---------------------------------------------------------------
    ap.extractExemplars();
    ap.printClusters(xs, ys);

    // Clean up generated coordinate arrays
    delete[] xs;
    delete[] ys;

    return 0;
}
