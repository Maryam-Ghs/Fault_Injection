#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

/* --------------------------------------------------------------
   Ant Colony Optimization for a Travelling Salesman Problem (TSP)
   --------------------------------------------------------------
   - Only int and float types are used.
   - All dynamic data are allocated on the heap with new/delete.
   - Input (graph) is generated inside the program and contains many
     edge‑case values (zero distances, very large distances, etc.).
   - The algorithm is written in a very verbose, step‑by‑step style.
   - Mathematical expressions are broken into temporary variables.
   - Helper functions are used to keep the structure clear.
   -------------------------------------------------------------- */

/* ---------- Helper function prototypes ---------- */
void generate_graph(int node_cnt, float *dist_matrix);
void initialise_pheromone(int node_cnt, float *pher_matrix);
void initialise_heuristic(int node_cnt, const float *dist_matrix, float *heur_matrix);
void run_aco(int node_cnt,
             const float *dist_matrix,
             float *pher_matrix,
             const float *heur_matrix,
             int ant_cnt,
             int iter_cnt,
             float alpha,
             float beta,
             float evap,
             float q_val,
             int *best_path,
             float *best_len);
void construct_solution(int node_cnt,
                        const float *pher_matrix,
                        const float *heur_matrix,
                        int ant_start,
                        int *tour,
                        float *tour_len,
                        float alpha,
                        float beta);
void update_pheromone(int node_cnt,
                      float *pher_matrix,
                      const int *best_path,
                      float best_len,
                      int ant_cnt,
                      const int **all_tours,
                      const float *all_lengths,
                      float evap,
                      float q_val);

/* ---------- Main entry point ---------- */
int main()
{
    /* Seed the random generator */
    std::srand((unsigned)std::time(0));

    /* Reverse‑adversarial problem definition */
    int node_cnt = 10;                    // increased size for adversarial pattern
    int ant_cnt  = 5;                     // fewer ants to keep runtime reasonable
    int iter_cnt = 30;                    // fewer iterations for practicality

    /* Parameters for the ACO algorithm (all float) */
    float alpha = 1.0f;                   // influence of pheromone
    float beta  = 5.0f;                   // influence of heuristic information
    float evap  = 0.5f;                   // pheromone evaporation rate
    float q_val = 100.0f;                 // pheromone deposit factor

    /* Allocate the distance matrix on the heap */
    float *dist_matrix = new float[node_cnt * node_cnt];
    generate_graph(node_cnt, dist_matrix);

    /* Allocate pheromone matrix and initialise it */
    float *pher_matrix = new float[node_cnt * node_cnt];
    initialise_pheromone(node_cnt, pher_matrix);

    /* Allocate heuristic matrix (1 / distance) */
    float *heur_matrix = new float[node_cnt * node_cnt];
    initialise_heuristic(node_cnt, dist_matrix, heur_matrix);

    /* Containers for the best tour found */
    int   *best_path = new int[node_cnt + 1];
    float  best_len  = 0.0f;

    /* Run the Ant Colony Optimisation */
    run_aco(node_cnt,
            dist_matrix,
            pher_matrix,
            heur_matrix,
            ant_cnt,
            iter_cnt,
            alpha,
            beta,
            evap,
            q_val,
            best_path,
            &best_len);

    /* Output the result */
    std::cout << "Best tour length : " << best_len << std::endl;
    std::cout << "Best tour order  : ";
    for (int i = 0; i <= node_cnt; ++i)
    {
        std::cout << best_path[i];
        if (i < node_cnt) std::cout << " -> ";
    }
    std::cout << std::endl;

    /* Clean up heap memory */
    delete [] dist_matrix;
    delete [] pher_matrix;
    delete [] heur_matrix;
    delete [] best_path;

    return 0;
}

/* ---------- Function definitions ---------- */

/* Generate a deterministic graph with descending distances and periodic zero‑distance edges.
   This creates an adversarial pattern for the heuristic (inverse distance) values. */
void generate_graph(int node_cnt, float *dist_matrix)
{
    const int max_dist = 1000;
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            if (i == j)
            {
                /* Distance from a node to itself is zero */
                dist_matrix[i * node_cnt + j] = 0.0f;
            }
            else
            {
                /* Create a deterministic descending pattern */
                int index = i * node_cnt + j;
                int descending = max_dist - (index % max_dist);

                /* Introduce zero‑distance edge‑cases at regular intervals */
                if ((i + j) % 7 == 0)
                {
                    dist_matrix[i * node_cnt + j] = 0.0f;
                }
                else
                {
                    dist_matrix[i * node_cnt + j] = static_cast<float>(descending);
                }
            }
        }
    }
}

/* Initialise all pheromone values to a small positive constant. */
void initialise_pheromone(int node_cnt, float *pher_matrix)
{
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            pher_matrix[i * node_cnt + j] = 0.1f;
        }
    }
}

/* Initialise heuristic information as the inverse of distance.
   If distance is zero, set heuristic to zero to avoid division by zero. */
void initialise_heuristic(int node_cnt, const float *dist_matrix, float *heur_matrix)
{
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            float d = dist_matrix[i * node_cnt + j];
            if (d <= 0.0f)
            {
                heur_matrix[i * node_cnt + j] = 0.0f;
            }
            else
            {
                heur_matrix[i * node_cnt + j] = 1.0f / d;
            }
        }
    }
}

/* Core ACO driver: iterates, constructs solutions, and updates pheromones. */
void run_aco(int node_cnt,
             const float *dist_matrix,
             float *pher_matrix,
             const float *heur_matrix,
             int ant_cnt,
             int iter_cnt,
             float alpha,
             float beta,
             float evap,
             float q_val,
             int *best_path,
             float *best_len)
{
    /* Allocate storage for all ants' tours and their lengths */
    int   **all_tours   = new int*[ant_cnt];
    float *all_lengths  = new float[ant_cnt];

    for (int a = 0; a < ant_cnt; ++a)
    {
        all_tours[a] = new int[node_cnt + 1];   // +1 to store return to start
    }

    /* Initialise best length with a large number */
    *best_len = 1e9f;

    /* Main optimisation loop */
    for (int it = 0; it < iter_cnt; ++it)
    {
        /* ----- Step 1: each ant builds a tour ----- */
        for (int a = 0; a < ant_cnt; ++a)
        {
            /* Choose a random start city for this ant */
            int start_city = std::rand() % node_cnt;

            /* Build the tour and compute its length */
            construct_solution(node_cnt,
                               pher_matrix,
                               heur_matrix,
                               start_city,
                               all_tours[a],
                               &all_lengths[a],
                               alpha,
                               beta);
        }

        /* ----- Step 2: find the best tour of this iteration ----- */
        int   *iteration_best_path = nullptr;
        float  iteration_best_len  = 1e9f;

        for (int a = 0; a < ant_cnt; ++a)
        {
            if (all_lengths[a] < iteration_best_len)
            {
                iteration_best_len  = all_lengths[a];
                iteration_best_path = all_tours[a];
            }
        }

        /* ----- Step 3: update global best if improved ----- */
        if (iteration_best_len < *best_len)
        {
            *best_len = iteration_best_len;

            for (int i = 0; i <= node_cnt; ++i)
            {
                best_path[i] = iteration_best_path[i];
            }
        }

        /* ----- Step 4: pheromone evaporation and deposit ----- */
        update_pheromone(node_cnt,
                         pher_matrix,
                         best_path,
                         *best_len,
                         ant_cnt,
                         (const int **)all_tours,
                         all_lengths,
                         evap,
                         q_val);
    }

    /* Clean up per‑ant tour storage */
    for (int a = 0; a < ant_cnt; ++a)
    {
        delete [] all_tours[a];
    }
    delete [] all_tours;
    delete [] all_lengths;
}

/* Construct a single ant's tour using probabilistic state transition rule. */
void construct_solution(int node_cnt,
                        const float *pher_matrix,
                        const float *heur_matrix,
                        int ant_start,
                        int *tour,
                        float *tour_len,
                        float alpha,
                        float beta)
{
    /* ----- Initialise visitation flags ----- */
    int *visited = new int[node_cnt];
    for (int i = 0; i < node_cnt; ++i)
    {
        visited[i] = 0;
    }

    /* ----- First city is the start city ----- */
    int current_city = ant_start;
    tour[0] = current_city;
    visited[current_city] = 1;

    /* ----- Build the rest of the tour ----- */
    for (int step = 1; step < node_cnt; ++step)
    {
        /* Compute denominator of the probability formula */
        float denom = 0.0f;
        for (int j = 0; j < node_cnt; ++j)
        {
            if (!visited[j])
            {
                float pher = pher_matrix[current_city * node_cnt + j];
                float heur = heur_matrix[current_city * node_cnt + j];

                float pow_pher = 1.0f;
                for (int p = 0; p < (int)alpha; ++p) pow_pher *= pher;   // α is small, loop ok

                float pow_heur = 1.0f;
                for (int p = 0; p < (int)beta; ++p) pow_heur *= heur;   // β is small, loop ok

                float term = pow_pher * pow_heur;
                denom += term;
            }
        }

        /* Choose next city based on roulette wheel selection */
        float r = ((float)std::rand()) / ((float)RAND_MAX);
        float cumulative = 0.0f;
        int chosen_city = -1;

        for (int j = 0; j < node_cnt; ++j)
        {
            if (!visited[j])
            {
                float pher = pher_matrix[current_city * node_cnt + j];
                float heur = heur_matrix[current_city * node_cnt + j];

                float pow_pher = 1.0f;
                for (int p = 0; p < (int)alpha; ++p) pow_pher *= pher;

                float pow_heur = 1.0f;
                for (int p = 0; p < (int)beta; ++p) pow_heur *= heur;

                float term = pow_pher * pow_heur;
                float prob = term / denom;

                cumulative += prob;
                if (r <= cumulative)
                {
                    chosen_city = j;
                    break;
                }
            }
        }

        /* Fallback in case of numerical issues */
        if (chosen_city == -1)
        {
            for (int j = 0; j < node_cnt; ++j)
            {
                if (!visited[j])
                {
                    chosen_city = j;
                    break;
                }
            }
        }

        /* Record the chosen city */
        tour[step] = chosen_city;
        visited[chosen_city] = 1;
        current_city = chosen_city;
    }

    /* ----- Return to the start city to complete the cycle ----- */
    tour[node_cnt] = ant_start;

    /* ----- Compute total length of the tour ----- */
    float total_len = 0.0f;
    for (int i = 0; i < node_cnt; ++i)
    {
        int from = tour[i];
        int to   = tour[i + 1];

        /* Retrieve distance via heuristic (since distance matrix not passed) */
        float heur = ((float*)heur_matrix)[from * node_cnt + to];
        float d = (heur > 0.0f) ? (1.0f / heur) : 0.0f;

        total_len += d;
    }

    *tour_len = total_len;

    /* Clean up visitation array */
    delete [] visited;
}

/* Evaporate pheromone globally and deposit pheromone based on the best tour. */
void update_pheromone(int node_cnt,
                      float *pher_matrix,
                      const int *best_path,
                      float best_len,
                      int ant_cnt,
                      const int **all_tours,
                      const float *all_lengths,
                      float evap,
                      float q_val)
{
    /* ----- Evaporation: multiply each pheromone value by (1 - evap) ----- */
    for (int i = 0; i < node_cnt; ++i)
    {
        for (int j = 0; j < node_cnt; ++j)
        {
            float old_val = pher_matrix[i * node_cnt + j];
            float new_val = old_val * (1.0f - evap);
            pher_matrix[i * node_cnt + j] = new_val;
        }
    }

    /* ----- Deposit pheromone from the best tour (global update) ----- */
    for (int i = 0; i < node_cnt; ++i)
    {
        int from = best_path[i];
        int to   = best_path[i + 1];

        /* Deposit amount = Q / best_len */
        float deposit = q_val / best_len;

        float old_val = pher_matrix[from * node_cnt + to];
        float new_val = old_val + deposit;
        pher_matrix[from * node_cnt + to] = new_val;

        /* Since the graph is undirected, also update the reverse edge */
        old_val = pher_matrix[to * node_cnt + from];
        new_val = old_val + deposit;
        pher_matrix[to * node_cnt + from] = new_val;
    }

    /* ----- Optional: additional deposit from all ants (local update) ----- */
    for (int a = 0; a < ant_cnt; ++a)
    {
        const int *tour = all_tours[a];
        float tour_len = all_lengths[a];

        /* Avoid division by zero */
        if (tour_len <= 0.0f) continue;

        float deposit = q_val / tour_len;

        for (int i = 0; i < node_cnt; ++i)
        {
            int from = tour[i];
            int to   = tour[i + 1];

            float old_val = pher_matrix[from * node_cnt + to];
            float new_val = old_val + deposit;
            pher_matrix[from * node_cnt + to] = new_val;

            /* Undirected update */
            old_val = pher_matrix[to * node_cnt + from];
            new_val = old_val + deposit;
            pher_matrix[to * node_cnt + from] = new_val;
        }
    }
}
