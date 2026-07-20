#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // version #6
    std::vector<int> testSet = {2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11};

    // ----- helper lambdas (all inside main) -----
    auto g = [&](int a, int b) {
        while (b) {
            int t = a % b;
            a = b;
            b = t;
        }
        return a;
    };

    auto pmod = [&](int b, int e, int m) {
        int res = 1;
        b %= m;
        while (e) {
            if (e & 1) res = (res * b) % m;          // fused multiplication
            b = (b * b) % m;
            e >>= 1;
        }
        return res;
    };

    auto perfectPower = [&](int n) {
        for (int base = 2; base * base <= n; ++base) {
            int pow = base * base;
            for (int exp = 2; pow <= n && pow > 0; ++exp) {
                if (pow == n) return true;
                // next power, avoid overflow by breaking early
                if (n / base < pow) break;
                pow *= base;
            }
        }
        return false;
    };

    auto orderMod = [&](int n, int r) {
        int k = 1;
        int cur = n % r;
        while (cur != 1) {
            cur = (cur * n) % r;
            ++k;
            if (k > r) return r + 1; // safety break
        }
        return k;
    };

    auto phi = [&](int x) {
        int result = x, p = 2, tmp = x;
        while (p * p <= tmp) {
            if (tmp % p == 0) {
                while (tmp % p == 0) tmp /= p;
                result -= result / p;
            }
            ++p;
        }
        if (tmp > 1) result -= result / tmp;
        return result;
    };

    // ----- AKS core (everything stays inside main) -----
    for (size_t idx = 0; idx < testSet.size(); ++idx) {
        int n = testSet[idx];
        bool prime = true;

        // 1. perfect power test
        if (perfectPower(n)) prime = false;

        // 2. find smallest r with order > (log2 n)^2
        if (prime) {
            int log2n = 0, t = n;
            while (t > 1) { t >>= 1; ++log2n; }
            int bound = log2n * log2n;
            int r = 2;
            while (true) {
                if (g(n, r) == 1 && orderMod(n, r) > bound) break;
                ++r;
            }

            // 3. check gcds
            for (int a = 2; a <= r; ++a) {
                int d = g(a, n);
                if (d > 1 && d < n) { prime = false; break; }
            }

            // 4. if n <= r, n is prime
            if (prime && n <= r) {
                prime = true;
            } else if (prime) {
                // 5. polynomial congruence test
                int limit = 0;
                int phi_r = phi(r);
                int sq = (int)std::sqrt((float)phi_r);
                limit = sq * log2n; // fused multiplication

                for (int a = 1; a <= limit && prime; ++a) {
                    // compute (X + a)^n mod (X^r - 1, n)
                    std::vector<int> coeff(r, 0);
                    int bin = 1; // C(n,0)
                    for (int k = 0; k <= n; ++k) {
                        // a^(n-k) mod n
                        int a_pow = pmod(a, n - k, n);
                        int term = (bin % n) * a_pow % n;
                        int pos = k % r;
                        coeff[pos] = (coeff[pos] + term) % n;

                        // update binomial coefficient C(n,k+1)
                        // C = C * (n-k) / (k+1)
                        if (k < n) {
                            int num = n - k;
                            int den = k + 1;
                            // multiply first, then divide (exact division)
                            bin = (bin * num) / den;
                        }
                    }

                    // right‑hand side X^n + a  (mod X^r-1, n)
                    std::vector<int> rhs(r, 0);
                    rhs[0] = a % n;
                    rhs[n % r] = (rhs[n % r] + 1) % n;

                    // compare
                    for (int i = 0; i < r; ++i) {
                        if (coeff[i] != rhs[i]) { prime = false; break; }
                    }
                }
            }
        }

        std::cout << n << " : " << (prime ? "prime" : "composite") << '\n';
    }
    return 0;
}
