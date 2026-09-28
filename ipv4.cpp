#include "ipv4.h"

#include <cctype>
#include <cstddef>

namespace {

// Only digits, '.', and ':' can ever be part of a candidate token.
bool isTokenChar(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ':';
}

bool isDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

// Parses one unsigned decimal field starting at pos, stopping at the first
// non-digit or at end. Enforces:
//   - at least 1 digit and at most maxDigits digits
//   - no leading zero unless the field is exactly "0"
//   - value <= maxValue
// The digit count is checked BEFORE a digit is accumulated, so the value can
// never exceed 99999 and cannot overflow.
// On success, advances pos past the field and stores the value.
bool parseField(const std::string& s, std::size_t& pos, std::size_t end,
                int maxDigits, long maxValue, long& value) {
    std::size_t start = pos;
    int digits = 0;
    long v = 0;

    while (pos < end && isDigit(s[pos])) {
        if (++digits > maxDigits) {
            return false;               // too many digits
        }
        v = v * 10 + (s[pos] - '0');    // manual digit accumulation
        ++pos;
    }

    if (digits == 0) {
        return false;                   // empty field
    }
    if (digits > 1 && s[start] == '0') {
        return false;                   // disallowed leading zero
    }
    if (v > maxValue) {
        return false;                   // out of range
    }

    value = v;
    return true;
}

// Validates the ENTIRE run s[begin, end) against the grammar
//   octet '.' octet '.' octet '.' octet [ ':' port ]
// No partial matches: every character in the run must be consumed.
bool validateToken(const std::string& s, std::size_t begin, std::size_t end,
                   unsigned long& address, int& port) {
    std::size_t pos = begin;
    unsigned long addr = 0;

    for (int i = 0; i < 4; ++i) {
        long octet = 0;
        if (!parseField(s, pos, end, 3, 255, octet)) {
            return false;
        }
        // Cast to unsigned long before shifting so octet << 24 cannot
        // overflow a signed int.
        addr = (addr << 8) | static_cast<unsigned long>(octet);

        if (i < 3) {
            if (pos >= end || s[pos] != '.') {
                return false;           // missing separator / too few octets
            }
            ++pos;
        }
    }

    int p = -1;
    if (pos < end) {
        if (s[pos] != ':') {
            return false;               // e.g. trailing '.' or fifth octet
        }
        ++pos;
        long portValue = 0;
        if (!parseField(s, pos, end, 5, 65535, portValue)) {
            return false;               // colon present => port must be valid
        }
        p = static_cast<int>(portValue);
        if (pos != end) {
            return false;               // anything after the port (':' or '.')
        }
    }

    address = addr;
    port = p;
    return true;
}

}  // namespace

bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0;
    outPort = -1;

    std::size_t i = 0;
    const std::size_t n = str.size();

    while (i < n) {
        if (!isTokenChar(str[i])) {
            ++i;                        // garbage: skip
            continue;
        }

        // Find the maximal run of token characters.
        std::size_t j = i;
        while (j < n && isTokenChar(str[j])) {
            ++j;
        }

        unsigned long addr = 0;
        int port = -1;
        if (validateToken(str, i, j, addr, port)) {
            outAddress = addr;
            outPort = port;
            return true;                // first valid token wins
        }

        i = j;                          // reject whole run, keep scanning
    }

    return false;
}
