#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

// compute squared Euclidean distance between two points (each point is a 2‑D vector)
float sqDist(const std::vector<float>& pA, const std::vector<float>& pB)
{
    float d0 = pA[0] - pB[0];
    float d1 = pA[1] - pB[1];
    return d0 * d0 + d1 * d1;          // reordered arithmetic, no double used
}

// create one‑element clusters from the list of points
void makeClusters(const std::vector<std::vector<float>>& pts,
                  std::vector<std::vector<int>>& cls)
{
    int i = 0;
    while (i < (int)pts.size())
    {
        std::vector<int> one;
        one.push_back(i);
        cls.push_back(one);
        i = i + 1;
    }
}

// find the pair of clusters with the smallest single‑link distance
void nearestPair(const std::vector<std::vector<int>>& cls,
                 const std::vector<std::vector<float>>& pts,
                 int& outA, int& outB)
{
    float best = std::numeric_limits<float>::max();
    outA = -1;
    outB = -1;

    int a = 0;
    while (a < (int)cls.size())
    {
        int b = a + 1;
        while (b < (int)cls.size())
        {
            // single‑link: distance between the closest two points of the clusters
            int i = 0;
            while (i < (int)cls[a].size())
            {
                int j = 0;
                while (j < (int)cls[b].size())
                {
                    float cur = sqDist(pts[cls[a][i]], pts[cls[b][j]]);
                    if (cur < best)
                    {
                        best = cur;
                        outA = a;
                        outB = b;
                    }
                    j = j + 1;
                }
                i = i + 1;
            }
            b = b + 1;
        }
        a = a + 1;
    }
}

// merge cluster idxB into idxA and erase idxB
void joinClusters(std::vector<std::vector<int>>& cls, int idxA, int idxB)
{
    int k = 0;
    while (k < (int)cls[idxB].size())
    {
        cls[idxA].push_back(cls[idxB][k]);
        k = k + 1;
    }
    cls.erase(cls.begin() + idxB);
}

// print clusters with their points
void showClusters(const std::vector<std::vector<int>>& cls,
                  const std::vector<std::vector<float>>& pts)
{
    int c = 0;
    while (c < (int)cls.size())
    {
        std::cout << "Cluster " << c << ": ";
        int p = 0;
        while (p < (int)cls[c].size())
        {
            int id = cls[c][p];
            std::cout << "(" << std::fixed << std::setprecision(1)
                      << pts[id][0] << "," << pts[id][1] << ") ";
            p = p + 1;
        }
        std::cout << "\n";
        c = c + 1;
    }
}

int main()
{
    // deterministic sparse‑skewed test vectors (2‑D points)
    std::vector<std::vector<float>> data;
    // dense cluster near the origin
    data.push_back({0.1f, 0.2f});
    data.push_back({0.2f, 0.1f});
    data.push_back({0.15f, 0.15f});
    data.push_back({0.05f, 0.25f});
    data.push_back({0.3f, 0.05f});
    // isolated far‑away points creating a sparse, skewed distribution
    data.push_back({1000.0f, 5.0f});
    data.push_back({-950.0f, -20.0f});
    data.push_back({500.0f, -800.0f});
    data.push_back({-400.0f, 900.0f});
    data.push_back({1200.0f, 1200.0f});
    data.push_back({-1100.0f, 1100.0f});
    data.push_back({800.0f, 0.0f});
    data.push_back({0.0f, -950.0f});
    data.push_back({-600.0f, -600.0f});
    data.push_back({650.0f, 650.0f});
    data.push_back({-300.0f, 300.0f});
    data.push_back({400.0f, -400.0f});
    data.push_back({-750.0f, 250.0f});
    data.push_back({250.0f, -750.0f});
    data.push_back({900.0f, 900.0f});
    data.push_back({-900.0f, -900.0f});
    data.push_back({100.0f, 100.0f}); // a mildly separate point

    // start with each point as its own cluster
    std::vector<std::vector<int>> clusters;
    makeClusters(data, clusters);

    // stop when we have exactly 3 clusters
    int desired = 3;
    while ((int)clusters.size() > desired)
    {
        int a = -1, b = -1;
        nearestPair(clusters, data, a, b);
        if (a < 0 || b < 0) break;      // safety
        // always merge the higher index into the lower to keep indices valid
        if (a < b)
            joinClusters(clusters, a, b);
        else
            joinClusters(clusters, b, a);
    }

    // output the final clustering
    showClusters(clusters, data);
    return 0;
}
