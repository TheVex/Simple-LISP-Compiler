#!/bin/bash

# Semantic Analyzer Test Runner Script
# This script runs all semantic analyzer tests and displays results

echo "========================================="
echo "Simple LISP Compiler - Semantic Analyzer"
echo "Test Suite Runner"
echo "========================================="
echo ""

# Check if executable exists
if [ ! -f "build/bin/slisp" ]; then
    echo "Error: Executable not found. Please build the project first:"
    echo "  mkdir build && cd build && cmake .. && make"
    exit 1
fi

SLISP="./build/bin/slisp"
PASSED=0
FAILED=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_file="$2"
    local expected_pattern="$3"

    echo "-----------------------------------"
    echo "Test: $test_name"
    echo "File: $test_file"
    echo "-----------------------------------"

    if [ ! -f "$test_file" ]; then
        echo "❌ FAILED: Test file not found"
        FAILED=$((FAILED + 1))
        echo ""
        return
    fi

    OUTPUT=$($SLISP "$test_file" 2>&1)

    if echo "$OUTPUT" | grep -q "$expected_pattern"; then
        echo "✅ PASSED"
        PASSED=$((PASSED + 1))
    else
        echo "❌ FAILED"
        echo "Expected pattern: $expected_pattern"
        echo "Output:"
        echo "$OUTPUT" | head -20
        FAILED=$((FAILED + 1))
    fi
    echo ""
}

# Section 1: Semantic Error Tests
echo ""
echo "========================================="
echo "SECTION 1: Semantic Error Detection"
echo "========================================="
echo ""

run_test "Break Outside Loop" \
    "tests/test_semantic_break_error.txt" \
    "break statement outside of loop"

run_test "Return Outside Function" \
    "tests/test_semantic_return_error.txt" \
    "return statement outside of function"

run_test "Function Arity Mismatch" \
    "tests/test_semantic_arity_error.txt" \
    "expects 2 arguments but got 3"

run_test "Undefined Function Warning" \
    "tests/test_semantic_undefined_func.txt" \
    "function or variable not defined"

# Section 2: Optimization Tests
echo ""
echo "========================================="
echo "SECTION 2: Optimization Tests"
echo "========================================="
echo ""

run_test "Constant Folding - Arithmetic" \
    "tests/test_opt_constant_folding.txt" \
    "INT(30)"

run_test "Unreachable Code Removal" \
    "tests/test_opt_unreachable_code.txt" \
    "No semantic errors found"

# Section 3: Valid Code Tests
echo ""
echo "========================================="
echo "SECTION 3: Valid Code Tests"
echo "========================================="
echo ""

run_test "Complex Valid Program" \
    "tests/test_semantic_valid.txt" \
    "No semantic errors found"

run_test "Comprehensive Test" \
    "tests/test_comprehensive.txt" \
    "No semantic errors found"

# Section 4: Original Parser Tests (Regression)
echo ""
echo "========================================="
echo "SECTION 4: Regression Tests"
echo "========================================="
echo ""

run_test "Original Test 1" \
    "tests/test1.txt" \
    "No semantic errors found"

run_test "Original Test 3" \
    "tests/test3.txt" \
    "Parsing completed successfully"

run_test "Original Test 9" \
    "tests/test9.txt" \
    "No semantic errors found"

# Section 5: Specific Optimization Checks
echo ""
echo "========================================="
echo "SECTION 5: Detailed Optimization Checks"
echo "========================================="
echo ""

echo "-----------------------------------"
echo "Test: Constant Folding - Detailed"
echo "-----------------------------------"
OUTPUT=$($SLISP tests/test_opt_constant_folding.txt 2>&1)

# Check for specific optimizations
if echo "$OUTPUT" | grep -q "INT(30)" && \
   echo "$OUTPUT" | grep -q "BOOL(true)" && \
   echo "$OUTPUT" | grep -q "BOOL(false)"; then
    echo "✅ PASSED - Multiple constant types folded correctly"
    PASSED=$((PASSED + 1))
else
    echo "❌ FAILED - Some constants not folded"
    FAILED=$((FAILED + 1))
fi
echo ""

echo "-----------------------------------"
echo "Test: Mixed Type Constant Folding"
echo "-----------------------------------"
OUTPUT=$($SLISP tests/test_comprehensive.txt 2>&1)

if echo "$OUTPUT" | grep -q "REAL(13.5)" && \
   echo "$OUTPUT" | grep -q "REAL(10)"; then
    echo "✅ PASSED - Mixed int/real arithmetic works"
    PASSED=$((PASSED + 1))
else
    echo "❌ FAILED - Mixed type promotion not working"
    FAILED=$((FAILED + 1))
fi
echo ""

# Summary
echo ""
echo "========================================="
echo "TEST SUMMARY"
echo "========================================="
echo "Tests Passed: $PASSED"
echo "Tests Failed: $FAILED"
echo "Total Tests:  $((PASSED + FAILED))"
echo ""

if [ $FAILED -eq 0 ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "⚠️  Some tests failed. Please review the output above."
    exit 1
fi
