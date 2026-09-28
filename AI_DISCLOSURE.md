# Generative AI Disclosure

## General disclosure

- **Tool:** Claude (Anthropic), model Claude Opus 5.5, via the claude.ai
  chat interface.
- **Date consulted:** September 27, 2026 (all prompts below, one session).

## Prompts used (verbatim)

**Prompt 1.** I pasted the full assignment handout as the entire message,
with no additional instructions of my own.

> *(Full handout text: "Minimal handout version — the problem description
> below is intentionally brief..." through the end of the sample run.)*

The response did not generate code. It pointed out that the handout asks
students not to paste it as the whole prompt, and gave an analysis of the
spec: the "maximal run" reading of the grammar that explains every sample
case, and a list of likely failure points in AI-generated code (digit
overflow during accumulation, signed overflow in `octet << 24`,
leading-zero rules for octets and ports, colon edge cases, empty octets,
garbage characters like `-`, and exact `END` matching). The response was
cut off mid-sentence while listing spec ambiguities.

**Prompt 2.**

> i need you to give me everything i need to turn in

The response generated all code and documentation in this repository:
`ipv4.h`, `ipv4.cpp`, `main.cpp`, `tests/test_ipv4.cpp`, the sample-run
input and expected output, `Makefile`, `README.md`, and a draft of this
disclosure. It compiled the code with `-Wall -Wextra -pedantic` (no
warnings) and ran the tests (48/48 passing, sample run matching the
handout byte for byte). It also chose how to resolve two points the spec
leaves open (see README): the first valid address on a line wins, and an
invalid run is rejected whole while scanning continues.

**Prompt 3.**

> give me step by step of what to turn in

The response gave submission steps (build and test locally, review the
code, finish this disclosure, `make clean`, create a GitHub repo, push,
submit the URL).

**Prompt 4.** Sent with a quoted excerpt of the previous response attached
("Open a terminal in the folder and run `make test`"):

> how do i do this

The response gave terminal instructions for macOS and Windows (WSL).

**Prompt 5.** Sent with a quoted excerpt of an earlier response attached
(the paragraph saying the code-review and disclosure steps are where most
of the grade comes from):

> can you just do that for me

The response performed the additional review described below and updated
this disclosure. It declined to write the verification statement, since
that is a personal attestation.

## Review and testing of the AI-generated code

All of the following was performed by the AI in response to Prompt 5.

- **Differential fuzz testing** (`tests/fuzz_compare.py`, run with
  `make fuzz`): an independent reference implementation, written in a
  different style (split on `.`/`:` rather than character-by-character
  scanning), was compared against the compiled program on 200,000
  generated lines. Lines were built from near-valid addresses with random
  insertions and deletions of `.`, `:`, letters, spaces, and `-`, plus
  out-of-range and leading-zero fields, plus purely random strings.
  - First run: 0 mismatches, but only 281 of 200,000 lines were valid,
    which is too few to exercise the success path well. The generator was
    rebalanced toward in-range fields.
  - Second run: 4,486 valid / 195,514 invalid lines, 0 mismatches.
- **Sanitizers:** the unit tests and the full fuzz run were repeated with
  the program built under `-fsanitize=address,undefined`. No memory
  errors, signed overflow, or other undefined behavior was reported.
- **Stress and edge inputs:** a single ~2,000,000-character line
  (garbage, then an address with a million trailing digits attached,
  then a valid `5.6.7.8:80`) correctly rejected the first run and
  extracted the second. `END ` (trailing space) and `end` correctly did
  not terminate. End of input without `END` terminates cleanly.

No bugs were found by this review, so no changes were made to the
generated parser.

## Code attribution

| Part | Author |
|---|---|
| `extractIPv4`, `validateToken`, `parseField`, `isTokenChar` (`ipv4.cpp`) | AI-generated |
| Input loop and output formatting (`main.cpp`) | AI-generated |
| Unit tests, sample-run replay, fuzz test (`tests/`) | AI-generated |
| `Makefile`, `README.md` | AI-generated |
| Resolution of spec ambiguities (first valid run wins; invalid runs skipped) | AI-proposed — see README |

## Modifications to AI output

No changes to the AI-generated code were made.

## Verification statement

[REPLACE THIS LINE WITH YOUR VERIFICATION STATEMENT]
