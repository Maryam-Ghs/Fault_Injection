/ * LLM input variant 1: minimal-boundary * /
 // Affine Transformation (3D) – version #8
 // All code resides inside main, uses heap allocation, manual unrolling, and deterministic data.

 #include <iostream>

 int main()
 {
     // ---------- allocate and fill source points (1 point, 3 coordinates) ----------
     int pointCount = 1;
     float* src = new float[pointCount * 3];   // [x0,y0,z0]

     // deterministic test vector (boundary values: zeros)
     src[0] = 0.0f; src[1] = 0.0f; src[2] = 0.0f;   // point 0

     // ---------- allocate and fill 4x4 affine matrix (row‑major) ----------
     float* trMat = new float[4 * 4];
     // identity transformation (boundary case)
     trMat[0] = 1.0f;  trMat[1] = 0.0f;  trMat[2] = 0.0f;  trMat[3] = 0.0f;
     trMat[4] = 0.0f;  trMat[5] = 1.0f;  trMat[6] = 0.0f;  trMat[7] = 0.0f;
     trMat[8] = 0.0f;  trMat[9] = 0.0f;  trMat[10] = 1.0f; trMat[11] = 0.0f;
     trMat[12] = 0.0f; trMat[13] = 0.0f; trMat[14] = 0.0f; trMat[15] = 1.0f;

     // ---------- allocate result array ----------
     float* dst = new float[pointCount * 3]; // transformed points

     // ---------- manual loop unrolling for point 0 ----------
     float x0 = src[0];
     float y0 = src[1];
     float z0 = src[2];
     float tx0 = trMat[0] * x0 + trMat[1] * y0 + trMat[2] * z0 + trMat[3];
     float ty0 = trMat[4] * x0 + trMat[5] * y0 + trMat[6] * z0 + trMat[7];
     float tz0 = trMat[8] * x0 + trMat[9] * y0 + trMat[10] * z0 + trMat[11];
     dst[0] = tx0;
     dst[1] = ty0;
     dst[2] = tz0;

     // ---------- output ----------
     std::cout << "Original -> Transformed\n";
     std::cout << "---------------------------\n";
     std::cout << src[0] << " " << src[1] << " " << src[2] << " -> "
               << dst[0] << " " << dst[1] << " " << dst[2] << "\n";

     // ---------- cleanup ----------
     delete[] src;
     delete[] trMat;
     delete[] dst;

     return 0;
 }
