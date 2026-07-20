# Deterministic single-fault ADAD/ADAF

This runtime injects at most one fault during a program execution. It supports:

| Mode | Mutation |
| --- | --- |
| `int_msb` | Toggle integer bit 31 |
| `int_lsb` | Toggle integer bit 0 |
| `int_middle` | Toggle integer bit 15 |
| `float_mantissa_middle` | Toggle IEEE-754 float bit 11 |
| `float_exponent_middle` | Toggle IEEE-754 float bit 26 |
| `boolean` | Invert a comparison result |

Use `-l` to discover dynamically reached locations and their occurrence counts.
Use `-p ID,1 -c OCCURRENCE -t MODE` for a deterministic faulty run. Injection
metadata is written to standard error as a line beginning with `FI_INJECT`.

The repository campaign is launched with `Scripts/Run_FI_once.sh`.
