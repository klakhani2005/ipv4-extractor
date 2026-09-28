// Unit tests for extractIPv4. Build and run with: make test
#include <iostream>
#include <string>

#include "../ipv4.h"

struct Case {
    const char* input;
    bool expectFound;
    unsigned long expectAddress;  // ignored when expectFound is false
    int expectPort;               // -1 means "no port"
    const char* why;
};

static const Case cases[] = {
    // --- Handout sample run ---
    {"connecting to 192.168.1.1 now", true, 3232235777UL, -1, "sample: embedded address"},
    {"server=10.0.0.255:8080end",     true, 167772415UL, 8080, "sample: address with port"},
    {"192a168.1.1.1",                 true, 2818638081UL, -1, "sample: garbage splits runs"},
    {"192.168.1.1.",                  false, 0, -1, "sample: trailing period"},
    {"Connection from 192.168.1.1 refused", true, 3232235777UL, -1, "sample: mid-sentence"},
    {"192.168.01.1",                  false, 0, -1, "sample: leading zero octet"},
    {"1.2.3.4:99999",                 false, 0, -1, "sample: port out of range"},
    {"12.34.56",                      false, 0, -1, "sample: three octets"},
    {"no number here",                false, 0, -1, "sample: no digits"},

    // --- Boundary values ---
    {"0.0.0.0",                       true, 0UL, -1, "all zeros"},
    {"255.255.255.255",               true, 4294967295UL, -1, "max address (shift overflow check)"},
    {"128.0.0.1",                     true, 2147483649UL, -1, "high bit set (signed shift bug)"},
    {"1.2.3.4:0",                     true, 16909060UL, 0, "port 0 allowed"},
    {"1.2.3.4:65535",                 true, 16909060UL, 65535, "max port"},
    {"1.2.3.4:65536",                 false, 0, -1, "port one over max"},
    {"256.1.1.1",                     false, 0, -1, "octet one over max"},
    {"1.2.3.999",                     false, 0, -1, "last octet out of range"},

    // --- Digit-count / overflow ---
    {"1.2.3.0004",                    false, 0, -1, "4-digit octet"},
    {"99999999999999999999.1.1.1",    false, 0, -1, "huge octet must not overflow"},
    {"1.2.3.4:999999999999",          false, 0, -1, "huge port must not overflow"},
    {"1.2.3.4:000001",                false, 0, -1, "6-digit port"},

    // --- Leading zeros ---
    {"01.2.3.4",                      false, 0, -1, "leading zero first octet"},
    {"1.2.3.00",                      false, 0, -1, "double zero octet"},
    {"10.20.30.40",                   true, 169090600UL, -1, "trailing zeros are fine"},
    {"1.2.3.4:080",                   false, 0, -1, "leading zero port"},
    {"1.2.3.4:00",                    false, 0, -1, "double zero port"},

    // --- Structural errors ---
    {"1..2.3",                        false, 0, -1, "empty octet"},
    {".1.2.3.4",                      false, 0, -1, "leading period"},
    {"1.2.3.4.5",                     false, 0, -1, "five octets (no truncation)"},
    {"1.2.3.4:",                      false, 0, -1, "colon with empty port"},
    {"1.2.3.4::80",                   false, 0, -1, "double colon"},
    {"1.2.3.4:80:",                   false, 0, -1, "trailing colon after port"},
    {"1.2.3.4:80:90",                 false, 0, -1, "second port"},
    {"1.2.3:4",                       false, 0, -1, "colon before fourth octet"},
    {":1.2.3.4",                      false, 0, -1, "leading colon"},
    {"1.2.3.4:80.",                   false, 0, -1, "period after port"},
    {"1.2.3.4.:80",                   false, 0, -1, "period before colon"},
    {"1234",                          false, 0, -1, "bare number"},
    {"...:::",                        false, 0, -1, "only separators"},
    {"",                              false, 0, -1, "empty line"},

    // --- Garbage handling / multiple runs ---
    {"-1.2.3.4",                      true, 16909060UL, -1, "minus sign is garbage"},
    {"ip=[1.2.3.4]",                  true, 16909060UL, -1, "brackets are garbage"},
    {"1.2.3.4 port 80",               true, 16909060UL, -1, "space before port breaks the run"},
    {"1.2.3.4: 80",                   false, 0, -1, "colon then space: whole run rejected"},
    {"1.2.3.4:99999 5.6.7.8",         true, 84281096UL, -1, "invalid run skipped, later run found"},
    {"1.2.3.4 and 5.6.7.8",           true, 16909060UL, -1, "two valid: first wins"},
    {"time 12:30:45 from 8.8.8.8",    true, 134744072UL, -1, "timestamp run rejected, address found"},
    {"\t 10.0.0.1:443 \t",            true, 167772161UL, 443, "surrounding whitespace"},
};

int main() {
    int passed = 0, failed = 0;

    for (const Case& c : cases) {
        unsigned long addr = 12345;   // sentinel values to confirm outputs are written
        int port = 12345;
        bool found = extractIPv4(c.input, addr, port);

        bool ok;
        if (c.expectFound) {
            ok = found && addr == c.expectAddress && port == c.expectPort;
        } else {
            // Spec: on failure, address = 0 and port = -1.
            ok = !found && addr == 0 && port == -1;
        }

        if (ok) {
            ++passed;
        } else {
            ++failed;
            std::cout << "FAIL [" << c.why << "] input=\"" << c.input << "\""
                      << " got found=" << found << " addr=" << addr
                      << " port=" << port << "\n";
        }
    }

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
