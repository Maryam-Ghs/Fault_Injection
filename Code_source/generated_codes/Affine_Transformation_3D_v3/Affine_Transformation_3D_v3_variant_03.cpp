#include <iostream>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// 3‑D Affine transformation class (heap‑based, verbose, step‑by‑step)
// ------------------------------------------------------------
class Transform3D
{
public:
    // 4×4 matrix stored in a 1‑D array (row‑major order)
    float* matrix;          // size = 16

    // Constructor – allocate matrix and initialise to identity
    Transform3D()
    {
        matrix = new float[16];
        // identity matrix
        int i = 0;
        while (i < 16)
        {
            matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;   // 0,5,10,15 are the diagonal entries
            ++i;
        }
    }

    // Destructor – free heap memory
    ~Transform3D()
    {
        delete[] matrix;
    }

    // ----------------------------------------------------------------
    // Set scaling factors (sx, sy, sz)
    // ----------------------------------------------------------------
    void setScale(float sx, float sy, float sz)
    {
        // build scaling matrix S
        float* S = new float[16];
        int i = 0;
        while (i < 16)
        {
            S[i] = 0.0f;
            ++i;
        }
        S[0]  = sx;
        S[5]  = sy;
        S[10] = sz;
        S[15] = 1.0f;

        // combine: matrix = matrix * S   (post‑multiply)
        multiply(S);
        delete[] S;
    }

    // ----------------------------------------------------------------
    // Set rotation about Z axis (angle in radians)
    // ----------------------------------------------------------------
    void setRotateZ(float theta)
    {
        // build rotation matrix Rz
        float* Rz = new float[16];
        int i = 0;
        while (i < 16)
        {
            Rz[i] = 0.0f;
            ++i;
        }
        float c = cosf(theta);
        float s = sinf(theta);
        Rz[0] =  c;   Rz[1] = -s;
        Rz[4] =  s;   Rz[5] =  c;
        Rz[10] = 1.0f;
        Rz[15] = 1.0f;

        // combine: matrix = matrix * Rz
        multiply(Rz);
        delete[] Rz;
    }

    // ----------------------------------------------------------------
    // Set translation (tx, ty, tz)
    // ----------------------------------------------------------------
    void setTranslate(float tx, float ty, float tz)
    {
        // build translation matrix T
        float* T = new float[16];
        int i = 0;
        while (i < 16)
        {
            T[i] = (i % 5 == 0) ? 1.0f : 0.0f;   // identity first
            ++i;
        }
        T[3]  = tx;
        T[7]  = ty;
        T[11] = tz;

        // combine: matrix = matrix * T
        multiply(T);
        delete[] T;
    }

    // ----------------------------------------------------------------
    // Multiply current matrix by another (post‑multiply)
    // ----------------------------------------------------------------
    void multiply(float* other)
    {
        float* result = new float[16];
        int row = 0;
        while (row < 4)
        {
            int col = 0;
            while (col < 4)
            {
                // fused expression for dot‑product of row and column
                float sum = 
                    matrix[row * 4 + 0] * other[0 * 4 + col] +
                    matrix[row * 4 + 1] * other[1 * 4 + col] +
                    matrix[row * 4 + 2] * other[2 * 4 + col] +
                    matrix[row * 4 + 3] * other[3 * 4 + col];
                result[row * 4 + col] = sum;
                ++col;
            }
            ++row;
        }

        // replace old matrix with result
        int i = 0;
        while (i < 16)
        {
            matrix[i] = result[i];
            ++i;
        }
        delete[] result;
    }

    // ----------------------------------------------------------------
    // Apply transformation to a single 3‑D point (x,y,z)
    // Returns a heap‑allocated array of 3 floats
    // ----------------------------------------------------------------
    float* apply(const float* point)
    {
        float* out = new float[3];

        // homogeneous coordinate w = 1
        float x = point[0];
        float y = point[1];
        float z = point[2];
        float w = 1.0f;

        // fused computation of each component
        float xp = 
            matrix[0] * x + matrix[1] * y + matrix[2] * z + matrix[3] * w;
        float yp = 
            matrix[4] * x + matrix[5] * y + matrix[6] * z + matrix[7] * w;
        float zp = 
            matrix[8] * x + matrix[9] * y + matrix[10] * z + matrix[11] * w;
        // w' is ignored because we keep it equal to 1 for affine transforms

        out[0] = xp;
        out[1] = yp;
        out[2] = zp;

        return out;
    }
};

// ------------------------------------------------------------
// Main – generate data, build transformation, apply and print
// ------------------------------------------------------------
int main()
{
    // ----- 1. Define a few 3‑D points (heap allocated) -----
    const int pointCount = 3;
    float* points = new float[pointCount * 3];
    // point 0 : (0, 0, 0)
    points[0] = 0.0f; points[1] = 0.0f; points[2] = 0.0f;
    // point 1 : (1, 0, 0)
    points[3] = 1.0f; points[4] = 0.0f; points[5] = 0.0f;
    // point 2 : (0, 1, 1)
    points[6] = 0.0f; points[7] = 1.0f; points[8] = 1.0f;

    // ----- 2. Build an affine transformation -----
    Transform3D tr;                // starts as identity
    tr.setScale(1.0f, 1.0f, 1.0f); // identity scaling
    tr.setRotateZ(0.0f);           // no rotation
    tr.setTranslate(0.0f, 0.0f, 0.0f); // no translation

    // ----- 3. Apply transformation to each point (while loop) -----
    int idx = 0;
    while (idx < pointCount)
    {
        // pointer to the current point (3 floats)
        float* src = points + idx * 3;
        float* dst = tr.apply(src);

        // ----- 4. Print original and transformed coordinates -----
        std::cout << "Point " << idx << " original : ("
                  << src[0] << ", " << src[1] << ", " << src[2] << ")\n";
        std::cout << "Point " << idx << " transformed : ("
                  << dst[0] << ", " << dst[1] << ", " << dst[2] << ")\n\n";

        delete[] dst;   // free temporary result
        ++idx;
    }

    // ----- 5. Clean up -----
    delete[] points;

    return 0;
}
