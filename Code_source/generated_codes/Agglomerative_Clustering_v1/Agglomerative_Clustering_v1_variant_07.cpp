#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

float sqDist(const std::vector<float>& pA, const std::vector<float>& pB)
{
    float d0 = pA[0] - pB[0];
    float d1 = pA[1] - pB[1];
    return d0 * d0 + d1 * d1;
}

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
    std::vector<std::vector<float>> data;
    // reverse‑ordered points forming a descending diagonal (adversarial layout)
    for (int i = 12; i >= 1; --i)
    {
        data.push_back({static_cast<float>(i), static_cast<float>(i)});
    }

    std::vector<std::vector<int>> clusters;
    makeClusters(data, clusters);

    int desired = 3;
    while ((int)clusters.size() > desired)
    {
        int a = -1, b = -1;
        nearestPair(clusters, data, a, b);
        if (a < 0 || b < 0) break;
        if (a < b)
            joinClusters(clusters, a, b);
        else
            joinClusters(clusters, b, a);
    }

    showClusters(clusters, data);
    return 0;
}
