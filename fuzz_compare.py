#!/usr/bin/env python3
"""
Differential fuzz test (TEST-ONLY; not part of the submitted C++ program).

Generates random input lines, runs them through ./ipv4_extract, and compares
each result against an independent reference implementation written in a
different style (split-based, using Python's int() -- which is fine here
because this is a test oracle, not the graded parser).

Usage (from repo root, after `make`):  python3 tests/fuzz_compare.py [N]
"""
import random
import subprocess
import sys

TOKEN = set("0123456789.:")
PROMPT = "Enter a string (or 'END' to quit): "


def field_ok(s, max_digits, max_value):
    return (s != "" and all(c in "0123456789" for c in s)
            and len(s) <= max_digits
            and not (len(s) > 1 and s[0] == "0")
            and int(s) <= max_value)


def check_run(run):
    if run.count(":") > 1:
        return None
    addr_part, colon, port_part = run.partition(":")
    octets = addr_part.split(".")
    if len(octets) != 4 or not all(field_ok(o, 3, 255) for o in octets):
        return None
    port = None
    if colon:
        if not field_ok(port_part, 5, 65535):
            return None
        port = int(port_part)
    value = 0
    for o in octets:
        value = value * 256 + int(o)
    return (".".join(octets), value, port)


def reference(line):
    runs, cur = [], ""
    for c in line:
        if c in TOKEN:
            cur += c
        else:
            if cur:
                runs.append(cur)
            cur = ""
    if cur:
        runs.append(cur)
    for r in runs:
        res = check_run(r)
        if res:
            addr, value, port = res
            return ("Extracted IPv4 address: %s (decimal value: %d, port: %s)"
                    % (addr, value, "none" if port is None else port))
    return "Invalid input: no valid IPv4 address found"


def rand_field(max_digits):
    kind = random.random()
    if kind < 0.5:
        # in-range value, so a good share of lines are actually valid
        return str(random.randint(0, 255 if max_digits == 3 else 65535))
    if kind < 0.7:
        return random.choice(["0", "00", "01", "255", "256", "65535", "65536",
                              "99999", "100000", "999"])
    return "".join(random.choice("0123456789")
                   for _ in range(random.randint(0, max_digits + 3)))


def rand_line():
    r = random.random()
    if r < 0.6:
        # near-valid address with random mutations
        s = ".".join(rand_field(3) for _ in range(random.choice([3, 4, 4, 4, 5])))
        if random.random() < 0.4:
            s += ":" + rand_field(5)
        chars = list(s)
        for _ in range(random.choice([0, 0, 1, 2])):
            op = random.random()
            pos = random.randint(0, len(chars))
            if op < 0.4:
                chars.insert(pos, random.choice(".:ab -"))
            elif op < 0.7 and chars:
                del chars[min(pos, len(chars) - 1)]
        s = "".join(chars)
        pre = "".join(random.choice("xy .:-9") for _ in range(random.randint(0, 3)))
        post = "".join(random.choice("xy .:-9") for _ in range(random.randint(0, 3)))
        return pre + s + post
    # fully random garbage over a biased alphabet
    return "".join(random.choice("0123456789..::ab -[]=")
                   for _ in range(random.randint(0, 40)))


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 200000
    random.seed(447)
    lines = []
    while len(lines) < n:
        line = rand_line()
        if line != "END":
            lines.append(line)
    stdin = "\n".join(lines) + "\nEND\n"
    out = subprocess.run(["./ipv4_extract"], input=stdin, capture_output=True,
                         text=True, check=True).stdout
    results = [chunk for chunk in out.split(PROMPT) if chunk][:-1]
    results = [r.rstrip("\n") for r in results]
    assert len(results) == n, "expected %d results, got %d" % (n, len(results))

    mismatches = 0
    for line, got in zip(lines, results):
        want = reference(line)
        if got != want:
            mismatches += 1
            if mismatches <= 10:
                print("MISMATCH input=%r\n  got:  %s\n  want: %s" % (line, got, want))
    valid = sum(1 for r in results if r.startswith("Extracted"))
    print("%d lines tested (%d valid, %d invalid), %d mismatches"
          % (n, valid, n - valid, mismatches))
    sys.exit(1 if mismatches else 0)


if __name__ == "__main__":
    main()
