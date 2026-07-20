#include <iostream>

/* LLM input variant 9: medium-deterministic-random */

//////////////////////////////////////////////////////////////
// Helper: generate two test strings with edge‑case patterns //
//////////////////////////////////////////////////////////////
void genTest(char *s1, char *s2, int &len1, int &len2)
{
    // Deterministic pseudo‑random generator (LCG)
    unsigned int state = 123456789u;
    auto next = [&state]() {
        state = (1103515245u * state + 12345u) & 0x7fffffff;
        return state;
    };

    // Choose a pattern index (0‑empty, 1‑single, 2‑repeating, 3‑deterministic mix)
    int pat = next() % 4;

    if (pat == 0)                     // both empty
    {
        len1 = 0; len2 = 0;
        s1[0] = '\0'; s2[0] = '\0';
        return;
    }
    if (pat == 1)                     // single character each (different from original)
    {
        len1 = 1; len2 = 1;
        s1[0] = 'M'; s2[0] = 'M';
        s1[1] = '\0'; s2[1] = '\0';
        return;
    }
    if (pat == 2)                     // long repeating strings with alternating chars
    {
        len1 = 92; len2 = 92;
        for (int i = 0; i < len1; ++i) s1[i] = (i % 2 == 0) ? 'Y' : 'Z';
        for (int i = 0; i < len2; ++i) s2[i] = (i % 2 == 0) ? 'Y' : 'Z';
        s1[len1] = '\0'; s2[len2] = '\0';
        return;
    }

    // pat == 3 : deterministic “random” mix, includes zeros and max length
    len1 = next() % 101;        // 0‑100
    len2 = next() % 101;
    for (int i = 0; i < len1; ++i) s1[i] = 'A' + (next() % 26);
    for (int i = 0; i < len2; ++i) s2[i] = 'A' + (next() % 26);
    s1[len1] = '\0'; s2[len2] = '\0';
}

//////////////////////////////////////////////////////////////
// Helper: compute global alignment score (Needleman‑Wunsch) //
//////////////////////////////////////////////////////////////
int globalScore(const char *a, const char *b, int n, int m)
{
    // DP matrix on the stack – maximum size 101×101
    int dp[101][101];

    // Scoring parameters (int only)
    int match = 1;
    int miss  = -1;
    int gap   = -2;

    // Initialise first row/column (gap penalties)
    int i = 0;
    while (i <= n) { dp[i][0] = i * gap; ++i; }
    int j = 0;
    while (j <= m) { dp[0][j] = j * gap; ++j; }

    // Fill matrix – using while loops and reordered arithmetic
    i = 1;
    while (i <= n)
    {
        j = 1;
        while (j <= m)
        {
            // Choose match or mismatch
            int subScore = (a[i-1] == b[j-1]) ? match : miss;

            // Compute three candidates
            int cand1 = dp[i-1][j-1] + subScore;   // diagonal
            int cand2 = dp[i-1][j]   - gap;        // up (gap in b)
            int cand3 = dp[i][j-1]   - gap;        // left (gap in a)

            // Reordered max: start with cand2, then compare cand1, cand3
            int best = cand2;
            if (cand1 > best) best = cand1;
            if (cand3 > best) best = cand3;

            dp[i][j] = best;
            ++j;
        }
        ++i;
    }
    return dp[n][m];
}

//////////////////////////////////////////////////////////////
// Helper: pretty‑print the test case and its alignment score //
//////////////////////////////////////////////////////////////
void report(const char *s1, const char *s2, int sc)
{
    std::cout << "String 1: \"" << s1 << "\"\n";
    std::cout << "String 2: \"" << s2 << "\"\n";
    std::cout << "Alignment score: " << sc << "\n";
    std::cout << "--------------------------\n";
}

//////////////////////////////////////////////////////////////
// Main driver – no external input, internal generation only //
//////////////////////////////////////////////////////////////
int main()
{
    // Stack buffers for the two strings (max length 100 + null)
    char buf1[101];
    char buf2[101];
    int lenA, lenB;

    // Generate a diverse edge‑case heavy test
    genTest(buf1, buf2, lenA, lenB);

    // Compute alignment
    int result = globalScore(buf1, buf2, lenA, lenB);

    // Output
    report(buf1, buf2, result);

    return 0;
}
