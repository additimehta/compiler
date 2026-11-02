#!/bin/bash

# WLP4 Type Checker Testing Script
# Generates .wlp4i from .wlp4, then compares .wlp4ti outputs

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0

# Check required tools
if ! command -v wlp4scan &> /dev/null; then
    echo -e "${RED}ERROR: wlp4scan not found in PATH${NC}"
    exit 1
fi

if ! command -v wlp4parse &> /dev/null; then
    echo -e "${RED}ERROR: wlp4parse not found in PATH${NC}"
    exit 1
fi

if ! command -v wlp4type &> /dev/null; then
    echo -e "${RED}ERROR: wlp4type not found in PATH${NC}"
    exit 1
fi

# Compile student implementation
if [ -f "wlp4type.cc" ]; then
    echo "Compiling wlp4type.cc..."
    if ! g++ -std=c++17 -Wall -o wlp4type_mine wlp4type.cc; then
        echo -e "${RED}Compilation failed!${NC}"
        exit 1
    fi
    IMPL="./wlp4type_mine"
    echo -e "${GREEN}✓ Compilation successful${NC}"
elif [ -f "wlp4type.rkt" ]; then
    IMPL="racket wlp4type.rkt"
    echo "Using Racket implementation"
else
    echo -e "${RED}ERROR: Neither wlp4type.cc nor wlp4type.rkt found!${NC}"
    exit 1
fi

echo ""
echo "================================"

# Function to test a single .wlp4 file
test_file() {
    local wlp4_file=$1
    local base=$(basename "$wlp4_file" .wlp4)
    local dir=$(dirname "$wlp4_file")
    
    # Skip empty test files
    if [ ! -s "$wlp4_file" ]; then
        echo -e "${YELLOW}Skipping empty test file: $base${NC}"
        return
    fi
    
    echo -e "${BLUE}Testing: $base${NC}"
    
    local out_dir="$dir"

    # Step 1: Parse to generate .wlp4i
    echo -n "  Parsing... "
    if ! wlp4scan < "$wlp4_file" | wlp4parse > "${out_dir}/${base}.wlp4i" 2> "${out_dir}/${base}.parse_err"; then
        echo -e "${RED}FAILED${NC}"
        echo "    Parse error:"
        cat "${out_dir}/${base}.parse_err" | head -5
        ((FAILED++))
        return
    fi
    echo -e "${GREEN}✓${NC}"
    
    # Step 2: Generate expected .wlp4ti
    echo -n "  Generating expected output... "
    wlp4type < "${out_dir}/${base}.wlp4i" > "${out_dir}/${base}.expected.wlp4ti" 2> "${out_dir}/${base}.expected.err"
    ref_exit=$?

    if [ $ref_exit -ne 0 ] || grep -q "ERROR" "${out_dir}/${base}.expected.err" 2>/dev/null || grep -q "ERROR" "${out_dir}/${base}.expected.wlp4ti" 2>/dev/null; then
        echo -e "${YELLOW}(expects error)${NC}"
        SHOULD_ERROR=1
    else
        echo -e "${GREEN}✓${NC}"
        SHOULD_ERROR=0
    fi
    
    # Step 3: Student output
    echo -n "  Running your type checker... "
    $IMPL < "${out_dir}/${base}.wlp4i" > "${out_dir}/${base}.wlp4ti" 2> "${out_dir}/${base}.actual.err"
    student_exit=$?

    if [ $student_exit -ne 0 ] || grep -q "ERROR" "${out_dir}/${base}.actual.err" 2>/dev/null || grep -q "ERROR" "${out_dir}/${base}.wlp4ti" 2>/dev/null; then

        if [ $SHOULD_ERROR -eq 1 ]; then
            echo -e "${GREEN}✓ (correctly detected error)${NC}"
            ((PASSED++))
            rm -f "${out_dir}/${base}.wlp4i" "${out_dir}/${base}.parse_err" \
                  "${out_dir}/${base}.expected.err" "${out_dir}/${base}.actual.err" \
                  "${out_dir}/${base}.expected.wlp4ti" "${out_dir}/${base}.wlp4ti"
            return
        else
            echo -e "${RED}FAILED${NC}"
            echo "    Your program errored but should succeed"
            ((FAILED++))
            return
        fi
    fi

    if [ $SHOULD_ERROR -eq 1 ]; then
        echo -e "${RED}FAILED${NC}"
        echo "    Your program succeeded but expected error"
        ((FAILED++))
        return
    fi
    
    echo -e "${GREEN}✓${NC}"
    
    echo -n "  Comparing outputs... "
    if diff -q "${out_dir}/${base}.expected.wlp4ti" "${out_dir}/${base}.wlp4ti" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ PASSED${NC}"
        ((PASSED++))
        rm -f "${out_dir}/${base}.wlp4i" "${out_dir}/${base}.parse_err" \
              "${out_dir}/${base}.expected.err" "${out_dir}/${base}.actual.err"
    else
        echo -e "${RED}FAILED (output mismatch)${NC}"
        ((FAILED++))
    fi
    
    echo ""
}

# Run tests
if [ -d "tests" ] && ls tests/*.wlp4 1> /dev/null 2>&1; then
    for file in tests/*.wlp4; do
        test_file "$file"
    done
fi

echo "================================"
echo "Test Summary:"
echo -e "  ${GREEN}Passed: $PASSED${NC}"
echo -e "  ${RED}Failed: $FAILED${NC}"
echo "  Total:  $((PASSED + FAILED))"
echo "================================"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some tests failed.${NC}"
    exit 1
fi
