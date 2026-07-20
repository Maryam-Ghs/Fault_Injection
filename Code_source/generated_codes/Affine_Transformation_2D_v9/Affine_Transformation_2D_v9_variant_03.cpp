/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cmath>

using namespace std;

//------------------------------------------------------------
// Class that stores a 2×3 affine matrix on the heap
//------------------------------------------------------------
class Affine2D {
public:
    float* mtx;               // [ a b tx ; c d ty ]  (size 6)

    // ctor – identity matrix
    Affine2D() {
        mtx = new float[6];
        mtx[0] = 1; mtx[1] = 0; mtx[2] = 0;
        mtx[3] = 0; mtx[4] = 1; mtx[5] = 0;
    }

    // dtor – free heap storage
    ~Affine2D() {
        delete[] mtx;
    }

    // set scaling matrix
    void setScale(float sx, float sy) {
        mtx[0] = sx; mtx[1] = 0;  mtx[2] = 0;
        mtx[3] = 0;  mtx[4] = sy; mtx[5] = 0;
    }

    // set rotation matrix (angle in radians)
    void setRotate(float ang) {
        float c = cosf(ang);
        float s = sinf(ang);
        mtx[0] = c;  mtx[1] = -s; mtx[2] = 0;
        mtx[3] = s;  mtx[4] =  c; mtx[5] = 0;
    }

    // set translation matrix
    void setTranslate(float tx, float ty) {
        mtx[0] = 1; mtx[1] = 0; mtx[2] = tx;
        mtx[3] = 0; mtx[4] = 1; mtx[5] = ty;
    }

    // combine “other” after the current transform:
    //   newM = other * this   (so points are first transformed by this,
    //   then by other)
    void combineAfter(const Affine2D& other) {
        float* res = new float[6];

        // left matrix (other)
        float aL = other.mtx[0]; float bL = other.mtx[1]; float txL = other.mtx[2];
        float cL = other.mtx[3]; float dL = other.mtx[4]; float tyL = other.mtx[5];

        // right matrix (this)
        float aR = mtx[0]; float bR = mtx[1]; float txR = mtx[2];
        float cR = mtx[3]; float dR = mtx[4]; float tyR = mtx[5];

        // temporary results
        float r00 = aL * aR + bL * cR;
        float r01 = aL * bR + bL * dR;
        float r02 = aL * txR + bL * tyR + txL;

        float r10 = cL * aR + dL * cR;
        float r11 = cL * bR + dL * dR;
        float r12 = cL * txR + dL * tyR + tyL;

        // store back
        res[0] = r00; res[1] = r01; res[2] = r02;
        res[3] = r10; res[4] = r11; res[5] = r12;

        delete[] mtx;
        mtx = res;
    }

    // apply the transformation to an array of points (x,y interleaved)
    void apply(const float* src, float* dst, int cnt) {
        int i = 0;
        while (i < cnt) {
            // read source point
            float px = src[2*i];
            float py = src[2*i + 1];

            // matrix components
            float a = mtx[0]; float b = mtx[1]; float tx = mtx[2];
            float c = mtx[3]; float d = mtx[4]; float ty = mtx[5];

            // compute transformed point (split into temporaries)
            float nx = a * px + b * py + tx;
            float ny = c * px + d * py + ty;

            // write destination point
            dst[2*i]     = nx;
            dst[2*i + 1] = ny;

            i = i + 1;      // while‑loop increment (reordered style)
        }
    }
};

//------------------------------------------------------------
// Helper functions to initialise specific transforms
//------------------------------------------------------------
void initScale(Affine2D& tr, float sx, float sy) {
    tr.setScale(sx, sy);
}
void initRotate(Affine2D& tr, float rad) {
    tr.setRotate(rad);
}
void initTranslate(Affine2D& tr, float tx, float ty) {
    tr.setTranslate(tx, ty);
}

//------------------------------------------------------------
// Main – deterministic test vectors, heap storage, printing
//------------------------------------------------------------
int main() {
    // deterministic square with zeros and ones
    int ptNum = 4;
    float* srcPts = new float[ptNum * 2];
    srcPts[0] = 0; srcPts[1] = 0;
    srcPts[2] = 1; srcPts[3] = 0;
    srcPts[4] = 1; srcPts[5] = 1;
    srcPts[6] = 0; srcPts[7] = 1;

    // Build individual transforms emphasizing zeros, ones, and identity
    Affine2D sc;  initScale(sc, 1.0f, 0.0f);          // scale x by 1, y by 0 (collapse y)
    Affine2D rt;  initRotate(rt, 0.0f);               // no rotation
    Affine2D tr;  initTranslate(tr, 0.0f, 0.0f);       // no translation

    // Combine: scale → rotate → translate
    Affine2D total;               // starts as identity
    total.combineAfter(sc);      // total = sc
    total.combineAfter(rt);      // total = rt * sc (still sc)
    total.combineAfter(tr);      // total = tr * rt * sc (still sc)

    // Apply combined transform
    float* dstPts = new float[ptNum * 2];
    total.apply(srcPts, dstPts, ptNum);

    // Print results
    int k = 0;
    while (k < ptNum) {
        cout << "P" << k << ": ("
             << srcPts[2*k] << ", " << srcPts[2*k + 1] << ") -> ("
             << dstPts[2*k] << ", " << dstPts[2*k + 1] << ")" << endl;
        k = k + 1;
    }

    // clean up heap memory
    delete[] srcPts;
    delete[] dstPts;
    return 0;
}
