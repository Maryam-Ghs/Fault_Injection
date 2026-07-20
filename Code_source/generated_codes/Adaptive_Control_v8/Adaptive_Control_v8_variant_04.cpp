/ * LLM input variant 4: signed-extremes * /
 // Adaptive Control implementation – version #8
 // ------------------------------------------------
 // This program simulates a simple discrete‑time adaptive controller.
 // All code resides inside main(), uses only float/int, std::vector and
 // deterministic test vectors. No external input is read.

 #include <vector>
 #include <cstdio>
 #include <cmath>

 int main()
 {
     // ------------------------------------------------
     // Simulation settings
     // ------------------------------------------------
     int steps = 120;                     // total simulation steps
     float a_real = -0.6f;               // true plant coefficient a (negative)
     float b_real = 0.7f;                // true plant coefficient b (positive)
     float a_ref  = 0.9f;                 // reference model coefficient a
     float b_ref  = -0.4f;               // reference model coefficient b (negative)
     float adapt_gain = 0.01f;           // adaptation gain γ

     // ------------------------------------------------
     // Containers for signals and parameters
     // ------------------------------------------------
     std::vector<float> r(steps);         // reference signal
     std::vector<float> y(steps);         // plant output
     std::vector<float> ym(steps);        // model output
     std::vector<float> u(steps);         // control input
     std::vector<float> th1(steps);      // adaptive weight for r
     std::vector<float> th2(steps);       // adaptive weight for y

     // ------------------------------------------------
     // Initial conditions
     // ------------------------------------------------
     r[0]  = 0.0f;        // start with zero reference
     y[0]  = 0.0f;
     ym[0] = 0.0f;
     th1[0] = 0.0f;
     th2[0] = 0.0f;

     // ------------------------------------------------
     // Deterministic reference trajectory (mixed sign)
     // ------------------------------------------------
     for (int i = 1; i < steps; ++i)
     {
         if (i < 20)
             r[i] = 0.0f;                                 // zero segment
         else if (i < 50)
             r[i] = -1.0f;                                // negative step
         else
         {
             // sinusoid that oscillates around zero, producing both
             // positive and negative values
             r[i] = sinf(0.15f * static_cast<float>(i));
         }
     }

     // ------------------------------------------------
     // Main adaptive loop (while‑style, heavily iterative)
     // ------------------------------------------------
     int k = 0;
     while (k < steps - 1)
     {
         // ---- Control law (current parameters) ----
         float ctrl = th1[k] * r[k] + th2[k] * y[k];
         u[k] = ctrl;

         // ---- Plant update (true dynamics) ----
         float y_next = a_real * y[k] + b_real * ctrl;
         y[k + 1] = y_next;

         // ---- Reference model update ----
         float ym_next = a_ref * ym[k] + b_ref * r[k];
         ym[k + 1] = ym_next;

         // ---- Error between plant and model ----
         float err = y[k + 1] - ym[k + 1];

         // ---- Regressor vector (expanded steps) ----
         float phi_r = r[k];          // regressor for reference
         float phi_y = y[k];          // regressor for plant output

         // ---- Parameter adaptation (multi‑step arithmetic) ----
         float delta_th1 = -adapt_gain * err * phi_r;
         float delta_th2 = -adapt_gain * err * phi_y;

         float th1_tmp = th1[k] + delta_th1;
         float th2_tmp = th2[k] + delta_th2;

         // ---- Store updated parameters ----
         th1[k + 1] = th1_tmp;
         th2[k + 1] = th2_tmp;

         // ---- Advance time index (reordered increment) ----
         k = k + 1;
     }

     // ------------------------------------------------
     // Output a subset of the simulation results
     // ------------------------------------------------
     for (int i = 0; i < steps; i += 15)
     {
         printf(
             "Step %2d | r=%.3f | y=%.3f | ym=%.3f | u=%.3f | th1=%.3f | th2=%.3f\n",
             i, r[i], y[i], ym[i], u[i], th1[i], th2[i]);
     }

     return 0;
 }
