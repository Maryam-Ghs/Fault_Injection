#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

//////////////////////////////////////////////////////////////
// Helper: generate two test strings with zeros‑and‑ones patterns //
//////////////////////////////////////////////////////////////
void genTest(char *s1, char *s2, int &len1, int &len2)
{
    // Deterministic pattern emphasizing zeros and ones
    // Long strings of length 100: first all '0's, second all '1's
    len1 = 100;
    len2 = 100;
    for (int i = 0; i < len1; ++i) s1[i] = '0';
    for (int i = 0; i < len2; ++i) s2[i] = '1';
    s1[len1] = '\0';
    s2[len2] = '\0';
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

    // Generate a zeros‑and‑ones heavy test
    genTest(buf1, buf2, lenA, lenB);

    // Compute alignment
    int result = globalScore(buf1, buf2, lenA, lenB);

    // Output
    report(buf1, buf2, result);

    return 0;
}
