#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper functions (all using int or float only)
// ------------------------------------------------------------
int gcd_int(int x, int y) {
    int a = x;
    int b = y;
    while (b != 0) {
        int temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

// fast exponentiation (base^exp mod mod)
int pow_mod_int(int base, int exp, int mod) {
    int result = 1;
    int b = base % mod;
    int e = exp;
    while (e > 0) {
        if (e % 2 == 1) {
            result = (result * b) % mod;
        }
        b = (b * b) % mod;
        e = e / 2;
    }
    return result;
}

// integer floor of log2(n)
int floor_log2_int(int n) {
    int cnt = 0;
    int t = n;
    while (t > 1) {
        t = t / 2;
        cnt = cnt + 1;
    }
    return cnt;
}

// integer square root (floor)
int int_sqrt(int n) {
    int low = 0;
    int high = n;
    int ans = 0;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (mid * mid <= n) {
            ans = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return ans;
}

// check if n is a perfect power
int is_perfect_power(int n) {
    int max_b = floor_log2_int(n) + 1;
    int b = 2;
    while (b <= max_b) {
        int low = 2;
        int high = n;
        while (low <= high) {
            int mid = (low + high) / 2;
            // compute mid^b (with overflow guard)
            int p = 1;
            int i = 0;
            while (i < b && p <= n) {
                p = p * mid;
                i = i + 1;
            }
            if (p == n) {
                return 1; // perfect power found
            } else if (p < n && p > 0) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        b = b + 1;
    }
    return 0;
}

// multiplicative order of n modulo r (small values only)
int order_mod_int(int n, int r) {
    int k = 1;
    int cur = n % r;
    while (cur != 1 && k <= r) {
        cur = (cur * n) % r;
        k = k + 1;
    }
    return cur == 1 ? k : r + 1; // return large value if order not found
}

// Euler's totient for small r (naive)
int phi_int(int r) {
    int count = 0;
    int i = 1;
    while (i <= r) {
        if (gcd_int(i, r) == 1) {
            count = count + 1;
        }
        i = i + 1;
    }
    return count;
}

// ------------------------------------------------------------
// AKS primality tester class
// ------------------------------------------------------------
class AKS_Prime_Tester {
public:
    // public interface
    int test(int candidate) {
        // step 1 : handle trivial cases
        if (candidate == 2) return 1;
        if (candidate < 2) return 0;
        if (candidate % 2 == 0) return 0;

        // step 2 : perfect power test
        if (is_perfect_power(candidate)) return 0;

        // step 3 : find smallest r with order > (log2 n)^2
        int log_n = floor_log2_int(candidate);
        int bound = log_n * log_n;
        int r = 2;
        while (r < candidate) {
            int ord = order_mod_int(candidate, r);
            if (ord > bound) break;
            r = r + 1;
        }

        // step 4 : check gcd for a <= r
        int a = 2;
        while (a <= r) {
            int g = gcd_int(candidate, a);
            if (g > 1 && g < candidate) return 0;
            a = a + 1;
        }

        // step 5 : if n <= r, it is prime
        if (candidate <= r) return 1;

        // step 6 : polynomial congruence check
        int limit = int_sqrt(phi_int(r)) * log_n;
        int a_idx = 1;
        while (a_idx <= limit) {
            if (!poly_congruence(candidate, r, a_idx)) {
                return 0;
            }
            a_idx = a_idx + 1;
        }

        // all tests passed -> prime
        return 1;
    }

private:
    // compute (X + a)^n mod (X^r-1, n) and compare with X^n + a
    int poly_congruence(int n, int r, int a) {
        // allocate coefficient arrays on the heap
        int* lhs = new int[r];
        int* rhs = new int[r];
        int i = 0;
        while (i < r) {
            lhs[i] = 0;
            rhs[i] = 0;
            i = i + 1;
        }

        // compute binomial coefficients iteratively
        int k = 0;
        int binom = 1; // C(n,0)
        while (k <= n) {
            int idx = k % r;
            lhs[idx] = (lhs[idx] + (binom % n)) % n;
            // update binom for next k
            if (k < n) {
                // binom = binom * (n - k) / (k + 1)
                // do multiplication first, then integer division
                int mult = binom * (n - k);
                int div = k + 1;
                binom = mult / div;
            }
            k = k + 1;
        }

        // RHS: X^n + a  (mod X^r-1, n)
        int pos_n = n % r;
        rhs[pos_n] = (rhs[pos_n] + 1) % n;
        rhs[0] = (rhs[0] + (a % n)) % n;

        // compare lhs and rhs coefficientwise
        int j = 0;
        while (j < r) {
            if (lhs[j] != rhs[j]) {
                delete[] lhs;
                delete[] rhs;
                return 0; // not congruent
            }
            j = j + 1;
        }

        delete[] lhs;
        delete[] rhs;
        return 1; // congruent
    }
};

// ------------------------------------------------------------
// Main driver – generate deterministic large array and test each entry
// ------------------------------------------------------------
int main() {
    // generate a large deterministic array (size 1000)
    const int array_len = 1000;
    int* numbers = new int[array_len];
    for (int i = 0; i < array_len; ++i) {
        // deterministic sequence: numbers from 2 upwards
        numbers[i] = i + 2;
    }

    // create tester object
    AKS_Prime_Tester* tester = new AKS_Prime_Tester();

    // run tests and print results
    for (int i = 0; i < array_len; ++i) {
        int val = numbers[i];
        int is_prime = tester->test(val);
        std::cout << "Number " << val << " is ";
        if (is_prime) {
            std::cout << "prime";
        } else {
            std::cout << "composite";
        }
        std::cout << std::endl;
    }

    // clean up heap memory
    delete[] numbers;
    delete tester;
    return 0;
}
