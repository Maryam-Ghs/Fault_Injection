#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

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
        int masked     = added & 0xFF;            // keep within a byte
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
    // Deterministic reverse‑ordered input: 20 descending values from 255
    int array_len = 20;
    std::vector<int> plain;
    for (int i = 0; i < array_len; ++i)
    {
        plain.push_back(255 - i);
    }

    // Adversarial keys: maximal XOR, maximal rotation, maximal addition, neutral second XOR
    int key_xor1 = 0xFF;          // flips all bits
    int key_rot  = 7;             // rotate by maximum 1‑7 bits
    int key_add  = 255;           // cause overflow to wrap to original
    int key_xor2 = 0x00;          // no change in final XOR

    // 3‑Way cipher stages
    std::vector<int> after_one = stage_one(plain, key_xor1);
    std::vector<int> after_two = stage_two(after_one, key_rot);
    std::vector<int> cipher    = stage_three(after_two, key_add, key_xor2);

    // Output
    display_vector(plain,      "Plaintext");
    std::cout << "Keys -> XOR1:" << key_xor1
              << " ROT:" << key_rot
              << " ADD:" << key_add
              << " XOR2:" << key_xor2 << '\n';
    display_vector(cipher, "Ciphertext");

    return 0;
}
