#!/bin/bash

# Directories fail
input_dir="tests/input_fail"
expected_dir="tests/expected_fail"
output_dir="tests/output_fail"
# Directories succ
input_dir_succ="tests/input"
expected_dir_succ="tests/expected"
output_dir_succ="tests/output"
# Directory .out
program="./hw5"

# Create output directory if it doesn't exist
mkdir -p "$output_dir"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
LIGHT_BLUE="\033[1;34m"
RESET="\033[0m"
BLUE='\033[0;34m'
YELLOW='\033[0;33m'

# Counters for summary
total_tests=0
passed_tests=0

# Iterate over each input file
echo -e "${YELLOW}This part checks the tests that fail when computing the LLVM IR.${NC}"

for input_file in "$input_dir"/*.in; do
    # Get the base name of the test (e.g., 1_valid)
    test_name=$(basename "$input_file" .in)

    # Expected and output file paths
    expected_file="$expected_dir/$test_name.exp"
    output_file="$output_dir/$test_name.out"

    # Run the program and redirect output
    "$program" < "$input_file" > "$output_file" 2>&1

    # Check if the expected file exists
    if [[ -f "$expected_file" ]]; then
        # Compare the output file with the expected file
        if diff -q "$output_file" "$expected_file" > /dev/null; then
            echo -e "$test_name: ${GREEN}PASS${NC}"
            ((passed_tests++))
        else
            echo -e "$test_name: ${RED}FAIL${NC}"
        fi
    else
        echo -e "${RED}Expected file not found for test: $test_name${NC}"
    fi

    # Increment total tests counter
    ((total_tests++))
done

# Summary
echo -e "\nSummary: $passed_tests out of $total_tests tests passed."
if [[ $passed_tests -eq $total_tests ]]; then
    echo -e "${GREEN}ALL TESTS PASSED!${NC}"
fi



# Create output directory if it doesn't exist
mkdir -p "$output_dir_succ"


# Counters for summary
total_tests_succ=0
passed_tests_succ=0

# Iterate over each input file
echo -e "${YELLOW}This part checks the tests that succeed when computing the LLVM IR.${NC}"

for input_file in "$input_dir_succ"/*.in; do
    # Get the base name of the test (e.g., 1_valid)
    test_name=$(basename "$input_file" .in)

    # Expected and output file paths
    expected_file="$expected_dir_succ/$test_name.exp"
    output_file="$output_dir_succ/$test_name.out"

    # Run the program and redirect output
	
    tmp_file="tmp.ll"
    touch "$tmp_file"
    "$program" < "$input_file" > "$tmp_file" 2>&1
    lli "$tmp_file" > "$output_file" 2>&1
    rm  "$tmp_file"

    # Check if the expected file exists
    if [[ -f "$expected_file" ]]; then
        # Compare the output file with the expected file
        if diff -q "$output_file" "$expected_file" > /dev/null; then
            echo -e "$test_name: ${GREEN}PASS${NC}"
            ((passed_tests_succ++))
        else
            echo -e "$test_name: ${RED}FAIL${NC}"
        fi
    else
        echo -e "${RED}Expected file not found for test: $test_name${NC}"
    fi

    # Increment total tests counter
    ((total_tests_succ++))
done

# Summary
echo -e "\nSummary: $passed_tests_succ out of $total_tests_succ tests passed."
if [[ $passed_tests_succ -eq $total_tests_succ ]]; then
    echo -e "${GREEN}ALL TESTS PASSED!${NC}"
fi
