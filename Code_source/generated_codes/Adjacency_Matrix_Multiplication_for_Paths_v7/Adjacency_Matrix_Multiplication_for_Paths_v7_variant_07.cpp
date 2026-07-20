/ * LLM input variant 7: reverse-adversarial * /
 // version #7 – Adjacency Matrix Multiplication for Paths
 // -----------------------------------------------------
 // Generates two deterministic adjacency matrices on the stack,
 // multiplies them to count length‑2 paths, and prints the result.
 // Only int is used; no const, unsigned, double, or long.

 #include <iostream>
 #include <cstdlib>
 #include <ctime>

 #define MAX_N 10   // maximum dimension, stack‑allocated

 // Fill a square matrix with random 0/1 values (directed graph)
 void buildRandom(int sz, int mat[][MAX_N])
 {
     for (int r = 0; r < sz; ++r)
         for (int c = 0; c < sz; ++c)
             mat[r][c] = std::rand() % 2;
 }

 // Multiply two adjacency matrices: res = left * right
 // Loop order i‑k‑j gives good cache behaviour and fuses the inner work.
 void multiply(int sz, int left[][MAX_N], int right[][MAX_N], int res[][MAX_N])
 {
     // initialise result to zero
     for (int i = 0; i < sz; ++i)
         for (int j = 0; j < sz; ++j)
             res[i][j] = 0;

     // core multiplication
     for (int i = 0; i < sz; ++i)
         for (int k = 0; k < sz; ++k)
         {
             int aik = left[i][k];
             // fuse the inner product into a single statement
             for (int j = 0; j < sz; ++j)
                 res[i][j] += aik * right[k][j];
         }
 }

 // Print a square matrix
 void show(int sz, int mat[][MAX_N])
 {
     for (int i = 0; i < sz; ++i)
     {
         for (int j = 0; j < sz; ++j)
             std::cout << mat[i][j] << ' ';
         std::cout << '\n';
     }
     std::cout << std::endl;
 }

 int main()
 {
     std::srand(static_cast<unsigned>(std::time(nullptr)));

     // Choose the maximum size to create a worst‑case dense graph
     int dim = MAX_N;                     // full size for adversarial test
     int adjA[MAX_N][MAX_N];
     int adjB[MAX_N][MAX_N];
     int paths[MAX_N][MAX_N];

     // Deterministically fill both adjacency matrices with all 1s
     for (int i = 0; i < dim; ++i)
         for (int j = 0; j < dim; ++j)
         {
             adjA[i][j] = 1;
             adjB[i][j] = 1;
         }

     std::cout << "Adjacency matrix A:\n";
     show(dim, adjA);

     std::cout << "Adjacency matrix B:\n";
     show(dim, adjB);

     // compute A * B – number of length‑2 paths from A to B
     multiply(dim, adjA, adjB, paths);

     std::cout << "Product matrix (paths of length 2):\n";
     show(dim, paths);

     return 0;
 }
