CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

all: ipv4_extract

ipv4_extract: main.cpp ipv4.cpp ipv4.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp ipv4.cpp

tests/test_ipv4: tests/test_ipv4.cpp ipv4.cpp ipv4.h
	$(CXX) $(CXXFLAGS) -o $@ tests/test_ipv4.cpp ipv4.cpp

test: ipv4_extract tests/test_ipv4
	./tests/test_ipv4
	./ipv4_extract < tests/sample_input.txt > tests/sample_actual.txt
	diff tests/sample_expected.txt tests/sample_actual.txt && echo "Sample run matches handout"

fuzz: ipv4_extract
	python3 tests/fuzz_compare.py 200000

clean:
	rm -f ipv4_extract tests/test_ipv4 tests/sample_actual.txt

.PHONY: all test fuzz clean
