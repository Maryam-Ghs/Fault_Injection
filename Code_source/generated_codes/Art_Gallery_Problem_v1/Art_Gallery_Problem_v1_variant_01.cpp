#include <iostream>
#include <cmath>    // only for std::sin / std::cos (float versions are fine)

// LLM input variant 1: minimal-boundary
class ArtGallery
{
public:
    int n;
    struct Point { float x; float y; } * verts;
    ArtGallery(int vertices) { n = vertices; verts = new Point[n]; generatePolygon(); }
    ~ArtGallery() { delete[] verts; }
    void generatePolygon()
    {
        if (n == 0) return;
        float step = 6.283185f / (float)n;
        float ang  = 0.0f;
        int i = 0;
        while (i + 3 < n)
        {
            verts[i    ].x = std::cos(ang);
            verts[i    ].y = std::sin(ang);
            ang += step;
            verts[i + 1].x = std::cos(ang);
            verts[i + 1].y = std::sin(ang);
            ang += step;
            verts[i + 2].x = std::cos(ang);
            verts[i + 2].y = std::sin(ang);
            ang += step;
            verts[i + 3].x = std::cos(ang);
            verts[i + 3].y = std::sin(ang);
            ang += step;
            i += 4;
        }
        while (i < n)
        {
            verts[i].x = std::cos(ang);
            verts[i].y = std::sin(ang);
            ang += step;
            ++i;
        }
    }
    int minGuards() const { return n / 3; }
};

int main()
{
    int testSizes[] = {0, 1, 2, 3};
    int testCount = sizeof(testSizes) / sizeof(testSizes[0]);
    int idx = 0;
    while (idx + 3 < testCount)
    {
        for (int k = 0; k < 4; ++k)
        {
            int sz = testSizes[idx + k];
            ArtGallery gallery(sz);
            std::cout << "Polygon with " << sz << " vertices needs "
                      << gallery.minGuards() << " guard(s).\n";
        }
        idx += 4;
    }
    while (idx < testCount)
    {
        int sz = testSizes[idx];
        ArtGallery gallery(sz);
        std::cout << "Polygon with " << sz << " vertices needs "
                  << gallery.minGuards() << " guard(s).\n";
        ++idx;
    }
    return 0;
}
