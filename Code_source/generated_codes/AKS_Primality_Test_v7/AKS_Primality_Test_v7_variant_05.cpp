// AKS Primality Test – version #7
// Implements the algorithm using only int and float types,
// heap‑allocated arrays, and helper functions.
/* LLM input variant 5: duplicate-heavy */

#include <iostream>

// ------------------------------------------------------------
// basic integer utilities
int gcd_int(int a, int b) {
    while (b) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int pow_mod_int(int base, int exp, int mod) {
    int res = 1 % mod;
    int cur = base % mod;
    while (exp > 0) {
        if (exp & 1) res = (res * cur) % mod;
        cur = (cur * cur) % mod;
        exp >>= 1;
    }
    return res;
}

// order of n modulo r
int order_mod(int n, int r) {
    int k = 1;
    int cur = n % r;
    while (cur != 1 && k <= r) {
        cur = (cur * n) % r;
        ++k;
    }
    return (cur == 1) ? k : r + 1;
}

// ------------------------------------------------------------
// step 1 – perfect power test
bool perfect_power(int n) {
    for (int base = 2; base * base <= n; ++base) {
        int p = base;
        while (p <= n) {
            p *= base;
            if (p == n) return true;
            if (p > n) break;
        }
    }
    return false;
}

// ------------------------------------------------------------
// polynomial multiplication (mod X^r‑1, mod n)
int* poly_mul_mod(int* A, int* B, int r, int mod) {
    int* C = new int[r];
    for (int i = 0; i < r; ++i) C[i] = 0;

    for (int i = 0; i < r; ++i) {
        if (A[i] == 0) continue;
        for (int j = 0; j < r; ++j) {
            if (B[j] == 0) continue;
            int idx = i + j;
            if (idx >= r) idx -= r;               // X^r ≡ 1
            C[idx] = (C[idx] + A[i] * B[j]) % mod;
        }
    }
    return C;
}

// binary exponentiation of a polynomial
int* poly_pow_mod(int* basePoly, int exp, int r, int mod) {
    int* result = new int[r];
    for (int i = 0; i < r; ++i) result[i] = 0;
    result[0] = 1 % mod;                         // polynomial “1”

    int* power = new int[r];
    for (int i = 0; i < r; ++i) power[i] = basePoly[i];

    int e = exp;
    while (e > 0) {
        if (e & 1) {
            int* tmp = poly_mul_mod(result, power, r, mod);
            delete[] result;
            result = tmp;
        }
        int* tmp2 = poly_mul_mod(power, power, r, mod);
        delete[] power;
        power = tmp2;
        e >>= 1;
    }
    delete[] power;
    return result;
}

// ------------------------------------------------------------
// step 5 – polynomial congruence check
bool congruence_check(int n, int r, int limit) {
    for (int a = 1; a <= limit; ++a) {
        // build (X + a)
        int* base = new int[r];
        for (int i = 0; i < r; ++i) base[i] = 0;
        base[0] = a % n;
        if (r > 1) base[1] = 1 % n;

        // compute (X + a)^n  (mod X^r‑1 , n)
        int* poly = poly_pow_mod(base, n, r, n);
        delete[] base;

        // expected coefficients: a at X^0, 1 at X^{n mod r}, 0 elsewhere
        int expIdx = n % r;
        bool ok = true;
        for (int i = 0; i < r; ++i) {
            int expected = 0;
            if (i == 0)          expected = a % n;
            else if (i == expIdx) expected = 1 % n;
            if (poly[i] != expected) { ok = false; break; }
        }
        delete[] poly;
        if (!ok) return false;
    }
    return true;
}

// ------------------------------------------------------------
// complete AKS test, returns 1 for prime, 0 for composite
int aks_test(int n) {
    if (n < 2) return 0;

    // step 1
    if (perfect_power(n)) return 0;

    // step 2 – find smallest r with order > (log₂ n)²
    int log2n = 0;
    for (int t = n; t > 1; t >>= 1) ++log2n;
    int bound = log2n * log2n;

    int r = 2;
    while (true) {
        if (gcd_int(n, r) == 1) {
            int ord = order_mod(n, r);
            if (ord > bound) break;
        }
        ++r;
    }

    // step 3 – gcd check
    for (int a = 2; a <= r && a < n; ++a) {
        if (gcd_int(a, n) > 1) return 0;
    }

    // step 4
    if (n <= r) return 1;

    // step 5 – polynomial test
    // limit = floor(sqrt(phi(r)) * log₂ n)  (approximate phi(r) ≤ r)
    int sqrtR = 0;
    for (int i = 0; i * i <= r; ++i) sqrtR = i;
    int limit = sqrtR * log2n;
    if (limit < 1) limit = 1;

    return congruence_check(n, r, limit) ? 1 : 0;
}

// ------------------------------------------------------------
int main() {
    // small predefined array of numbers to test, intentionally heavy on duplicates
    int size = 20;
    int* numbers = new int[size];
    int preset[20] = {
        2, 2,      // duplicate prime
        3, 3,      // duplicate prime
        4, 4,      // duplicate composite
        5, 5, 5,   // triple prime
        6, 6,      // duplicate composite
        7, 7,      // duplicate prime
        8, 8,      // duplicate composite
        9, 9,      // duplicate composite
        10, 10,    // duplicate composite
        11         // single prime
    };
    for (int i = 0; i < size; ++i) numbers[i] = preset[i];

    for (int i = 0; i < size; ++i) {
        int n = numbers[i];
        int ans = aks_test(n);
        if (ans)
            std::cout << n << " is prime\n";
        else
            std::cout << n << " is composite\n";
    }

    delete[] numbers;
    return 0;
}
