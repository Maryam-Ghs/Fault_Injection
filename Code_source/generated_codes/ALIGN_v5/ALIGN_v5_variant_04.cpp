#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

//////////////////////////////////////////////////////////////
// Helper: generate two test strings with edge‑case patterns //
//////////////////////////////////////////////////////////////
void genTest(char *s1, char *s2, int &len1, int &len2)
{
    // Seed for reproducibility
    std::srand(84);

    // Choose a pattern index (0‑empty, 1‑single, 2‑repeating, 3‑random)
    int pat = std::rand() % 4;

    if (pat == 0)                     // both empty
    {
        len1 = 0; len2 = 0;
        s1[0] = '\0'; s2[0] = '\0';
        return;
    }
    if (pat == 1)                     // single character each
    {
        len1 = 1; len2 = 1;
        s1[0] = 'Z'; s2[0] = 'Z';
        s1[1] = '\0'; s2[1] = '\0';
        return;
    }
    if (pat == 2)                     // long repeating strings
    {
        len1 = 100; len2 = 100;
        for (int i = 0; i < len1; ++i) s1[i] = '~';
        for (int i = 0; i < len2; ++i) s2[i] = '~';
        s1[len1] = '\0'; s2[len2] = '\0';
        return;
    }

    // pat == 3 : random mix, includes zeros and max length
    len1 = std::rand() % 101;        // 0‑100
    len2 = std::rand() % 101;
    for (int i = 0; i < len1; ++i) s1[i] = static_cast<char>('A' + (std::rand() % 26));
    for (int i = 0; i < len2; ++i) s2[i] = static_cast<char>('a' + (std::rand() % 26));
    s1[len1] = '\0'; s2[len2] = '\0';
}

//////////////////////////////////////////////////////////////
// Helper: compute global alignment score (Needleman‑Wunsch) //
//////////////////////////////////////////////////////////////
int globalScore(const char *a, const char *b, int n, int m)
{
    // DP matrix on the stack – maximum size 101×101
    int dp[101][101];

    // Scoring parameters (int only) – mixed negative, zero, positive extremes
    int match = 100;   // strong positive reward
    int miss  = 0;     // neutral mismatch
    int gap   = -100;  // strong negative penalty

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
