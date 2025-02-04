#!/bin/bash

# Directories fail
input_dir="TestMaster/input_fail"
expected_dir="TestMaster/expected_fail"
output_dir="TestMaster/output_fail"
# Directories succ
input_dir_succ="TestMaster/input"
expected_dir_succ="TestMaster/expected"
output_dir_succ="TestMaster/output"
# Directory .out
program="./hw5"

# Create output directories if they don't exist
mkdir -p "$output_dir"
mkdir -p "$output_dir_succ"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
YELLOW='\033[0;33m'

# Counters for summary
total_tests=0
passed_tests=0

echo -e "${YELLOW}This part checks the tests that fail when computing the LLVM IR.${NC}"

for input_file in "$input_dir"/*.in; do
    test_name=$(basename "$input_file" .in)
    expected_file="$expected_dir/$test_name.exp"
    output_file="$output_dir/$test_name.out"
    ir_file="$output_dir/$test_name.ll"

    "$program" < "$input_file" > "$ir_file" 2>&1

    if [[ -f "$expected_file" ]]; then
        if diff -q "$ir_file" "$expected_file" > /dev/null; then
            echo -e "$test_name: ${GREEN}PASS${NC}"
            ((passed_tests++))
        else
            echo -e "$test_name: ${RED}FAIL${NC}"
        fi
    else
        echo -e "${RED}Expected file not found for test: $test_name${NC}"
    fi
    ((total_tests++))
done

echo -e "\nSummary: $passed_tests out of $total_tests tests passed."
if [[ $passed_tests -eq $total_tests ]]; then
    echo -e "${GREEN}ALL TESTS PASSED!${NC}"
fi

# Counters for summary
total_tests_succ=0
passed_tests_succ=0

echo -e "${YELLOW}This part checks the tests that succeed when computing the LLVM IR.${NC}"

for input_file in "$input_dir_succ"/*.in; do
    test_name=$(basename "$input_file" .in)
    expected_file="$expected_dir_succ/$test_name.exp"
    output_file="$output_dir_succ/$test_name.out"
    ir_file="$output_dir_succ/$test_name.ll"

    "$program" < "$input_file" > "$ir_file" 2>&1
    lli "$ir_file" > "$output_file" 2>&1

    if [[ -f "$expected_file" ]]; then
        if diff -q "$output_file" "$expected_file" > /dev/null; then
            echo -e "$test_name: ${GREEN}PASS${NC}"
            ((passed_tests_succ++))
        else
            echo -e "$test_name: ${RED}FAIL${NC}"
        fi
    else
        echo -e "${RED}Expected file not found for test: $test_name${NC}"
    fi
    ((total_tests_succ++))
done

echo -e "\nSummary: $passed_tests_succ out of $total_tests_succ tests passed."
if [[ $passed_tests_succ -eq $total_tests_succ ]]; then
    echo -e "${GREEN}ALL TESTS PASSED!${NC}"
fi
