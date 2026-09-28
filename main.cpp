#include <iostream>
#include <string>

#include "ipv4.h"

int main() {
    std::string line;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";

        if (!std::getline(std::cin, line)) {
            // End of input (e.g. Ctrl-D or piped file ended) without END.
            std::cout << "\nProgram terminated." << std::endl;
            return 0;
        }

        // Tolerate Windows line endings in piped input files.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "END") {
            std::cout << "Program terminated." << std::endl;
            return 0;
        }

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(line, address, port)) {
            std::cout << "Extracted IPv4 address: "
                      << ((address >> 24) & 0xFFUL) << '.'
                      << ((address >> 16) & 0xFFUL) << '.'
                      << ((address >> 8) & 0xFFUL) << '.'
                      << (address & 0xFFUL)
                      << " (decimal value: " << address << ", port: ";
            if (port == -1) {
                std::cout << "none";
            } else {
                std::cout << port;
            }
            std::cout << ")" << std::endl;
        } else {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }
}
