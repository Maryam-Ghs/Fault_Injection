#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// Helper: create a vector with random integers (0‑255)
// ------------------------------------------------------------
std::vector<int> build_random_data(int length)
{
    std::vector<int> container;
    int index = 0;
    while (index < length)
    {
        int raw   = std::rand() % 256;   // value in byte range
        container.push_back(raw);
        ++index;
    }
    return container;
}

// ------------------------------------------------------------
// Helper: first cipher stage – simple XOR with a key
// ------------------------------------------------------------
std::vector<int> stage_one(const std::vector<int>& source, int xor_key)
{
    std::vector<int> out;
    for (size_t pos = 0; pos < source.size(); ++pos)
    {
        int original   = source[pos];
        int mixed      = original ^ xor_key;   // step 1
        out.push_back(mixed);
    }
    return out;
}

// ------------------------------------------------------------
// Helper: second cipher stage – left rotate by a shift amount
// ------------------------------------------------------------
int rotate_left_one_byte(int value, int shift)
{
    // isolate low 8 bits
    int low_part   = value & 0xFF;
    // split into two portions
    int left_part  = (low_part << shift) & 0xFF;
    int right_part = low_part >> (8 - shift);
    // recombine
    int rotated    = left_part | right_part;
    return rotated;
}

std::vector<int> stage_two(const std::vector<int>& source, int rot_shift)
{
    std::vector<int> out;
    int cursor = 0;
    while (cursor < static_cast<int>(source.size()))
    {
        int current    = source[cursor];
        int step_a     = current;               // keep original for clarity
        int step_b     = rotate_left_one_byte(step_a, rot_shift);
        out.push_back(step_b);
        ++cursor;
    }
    return out;
}

// ------------------------------------------------------------
// Helper: third cipher stage – add a constant then XOR again
// ------------------------------------------------------------
std::vector<int> stage_three(const std::vector<int>& source, int add_val, int xor_key)
{
    std::vector<int> out;
    for (size_t i = 0; i < source.size(); ++i)
    {
        int base       = source[i];
        int added      = base + add_val;          // addition first
        int masked     = added & 0xFF;             // keep within a byte
        int final_val  = masked ^ xor_key;        // then XOR
        out.push_back(final_val);
    }
    return out;
}

// ------------------------------------------------------------
// Helper: print vector contents as a space‑separated list
// ------------------------------------------------------------
void display_vector(const std::vector<int>& data, const char* label)
{
    std::cout << label << ": ";
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::cout << data[i];
        if (i + 1 < data.size())
            std::cout << ' ';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Main – orchestrates the 3‑Way Cipher
// ------------------------------------------------------------
int main()
{
    // seed random generator (kept for completeness)
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 1️⃣ generate a deterministic array emphasizing zeros and ones
    int array_len = 20;
    std::vector<int> plain(array_len);
    for (int i = 0; i < array_len; ++i)
    {
        plain[i] = (i < array_len / 2) ? 0 : 1; // first half zeros, second half ones
    }

    // 2️⃣ define deterministic keys emphasizing identity values
    int key_xor1 = 0;          // XOR with 0 leaves data unchanged
    int key_rot  = 1;          // minimal rotate (still valid)
    int key_add  = 0;          // add 0 leaves data unchanged
    int key_xor2 = 0;          // XOR with 0 leaves data unchanged

    // 3️⃣ stage one – XOR
    std::vector<int> after_one = stage_one(plain, key_xor1);

    // 4️⃣ stage two – rotate left
    std::vector<int> after_two = stage_two(after_one, key_rot);

    // 5️⃣ stage three – add then XOR
    std::vector<int> cipher = stage_three(after_two, key_add, key_xor2);

    // 6️⃣ output everything
    display_vector(plain,      "Plaintext");
    std::cout << "Keys -> XOR1:" << key_xor1
              << " ROT:" << key_rot
              << " ADD:" << key_add
              << " XOR2:" << key_xor2 << '\n';
    display_vector(cipher, "Ciphertext");

    return 0;
}
