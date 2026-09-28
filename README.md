# IPv4 Address Extractor (C++)

Reads lines of text and extracts a single valid IPv4 address, optionally
followed by a port, embedded anywhere in the line. All parsing is done
character by character; no `atoi`/`strtol`/`stoi`/`sscanf`, no `inet_*`
functions, and no regex.

## Build and run

```
make                 # builds ./ipv4_extract
./ipv4_extract       # interactive; type END to quit
make test            # unit tests + replay of the handout's sample run
make fuzz            # optional: 200,000-line differential fuzz test (needs python3)
```

Requires a C++17 compiler (tested with g++).

## Files

| File | Purpose |
|---|---|
| `ipv4.h` / `ipv4.cpp` | `extractIPv4` and its helpers |
| `main.cpp` | Input loop and output formatting |
| `tests/test_ipv4.cpp` | 48 unit tests (boundaries, leading zeros, overflow, malformed structure, garbage handling) |
| `tests/sample_input.txt` / `tests/sample_expected.txt` | Handout sample run, compared byte-for-byte by `make test` |
| `tests/fuzz_compare.py` | Test-only differential fuzzer: compares the program against an independent Python reference on random inputs. Not part of the graded parser. |
| `AI_DISCLOSURE.md` | Generative AI usage disclosure |

## How the parser works

1. **Tokenize.** Scan the line for maximal runs of token characters
   (digits, `.`, `:`). Every other character is garbage and ends a run.
2. **Validate each run in full** against
   `octet.octet.octet.octet[:port]`. Every character of the run must be
   consumed; there is no truncation to find a valid piece inside a longer
   run. This is why `192.168.1.1.` is rejected (the trailing period is in
   the same run) while `192a168.1.1.1` yields `168.1.1.1` (the `a` splits
   the line into runs `192` and `168.1.1.1`).
3. **Fields.** Octets are 1–3 digits, 0–255; ports are 1–5 digits,
   0–65535. Neither may have a leading zero unless the value is exactly 0.
   The digit count is checked before each digit is accumulated, so very
   long digit strings cannot overflow.
4. **Address value** is built as `(addr << 8) | octet` in `unsigned long`,
   avoiding the signed-int overflow of `octet << 24` for octets ≥ 128.

## Design decisions on points the spec leaves open

- **Multiple valid addresses on one line:** the first valid run is
  extracted (`1.2.3.4 and 5.6.7.8` → `1.2.3.4`).
- **An invalid run followed by a valid run:** the invalid run is rejected
  as a whole and scanning continues (`1.2.3.4:99999 5.6.7.8` → `5.6.7.8`).
  This is consistent with the handout's `192a168.1.1.1` example.
- **Characters like `-`, `[`, `]`, spaces** are garbage per the spec, so
  `-1.2.3.4` extracts `1.2.3.4` and `1.2.3.4: 80` is rejected (the run is
  `1.2.3.4:` with an empty port).
- **End of input without `END`** (e.g. Ctrl-D): the program prints
  `Program terminated.` and exits rather than looping forever.
- **Trailing `\r`** (Windows line endings in piped files) is stripped, so
  `END\r` terminates.
