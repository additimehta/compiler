#!/bin/bash

INPUT=${1:-input.wlp4}
BASE=$(basename "$INPUT" .wlp4)

echo "=== Testing: $INPUT ==="

# Step 1: Generate type-annotated parse tree
echo "Step 1: Type checking..."
wlp4scan < "$INPUT" | wlp4parse | wlp4type > "${BASE}.wlp4ti"
if [ $? -ne 0 ]; then
    echo "✗ Type checking failed!"
    exit 1
fi
echo "✓ Type checking passed"

# Step 2: Compile your code generator
echo "Step 2: Compiling code generator..."
g++ -std=c++17 -Wall -o wlp4gen wlp4gen.cc
if [ $? -ne 0 ]; then
    echo "✗ Compilation failed!"
    exit 1
fi
echo "✓ Code generator compiled"

# Step 3: Generate your assembly
echo "Step 3: Generating assembly..."
./wlp4gen < "${BASE}.wlp4ti" > "${BASE}.asm"
if [ $? -ne 0 ]; then
    echo "✗ Code generation failed!"
    exit 1
fi
echo "✓ Assembly generated"

# Step 4: Assemble your code
echo "Step 4: Assembling to machine code..."
cs241.binasm < "${BASE}.asm" > "${BASE}.mips"
if [ $? -ne 0 ]; then
    echo "✗ Assembly failed!"
    exit 1
fi
echo "✓ Assembled successfully"

# Step 5: Generate expected output with wlp4c
echo "Step 5: Generating expected output with wlp4c..."
wlp4c < "$INPUT" > "${BASE}-expected.mips"
if [ $? -ne 0 ]; then
    echo "✗ wlp4c failed!"
    exit 1
fi
echo "✓ Expected output generated"

# Step 6: Compare behavior
echo ""
echo "=== Step 6: Comparing behavior with input: 5 3 ==="
echo ""

echo "Your output:"
ACTUAL_OUTPUT=$(echo "5 3" | mips.twoints "${BASE}.mips" 2>&1)
echo "$ACTUAL_OUTPUT"

echo ""
echo "Expected output:"
EXPECTED_OUTPUT=$(echo "5 3" | mips.twoints "${BASE}-expected.mips" 2>&1)
echo "$EXPECTED_OUTPUT"

echo ""
echo "=== Checking register \$3 ==="
ACTUAL_REG3=$(echo "5 3" | mips.array "${BASE}.mips" 2>/dev/null | grep "^\$3" | awk '{print $2}')
EXPECTED_REG3=$(echo "5 3" | mips.array "${BASE}-expected.mips" 2>/dev/null | grep "^\$3" | awk '{print $2}')

echo "Your \$3: $ACTUAL_REG3"
echo "Expected \$3: $EXPECTED_REG3"

if [ "$ACTUAL_REG3" == "$EXPECTED_REG3" ]; then
    echo ""
    echo "✓ ALL TESTS PASSED! Return values match."
else
    echo ""
    echo "✗ FAIL: Return values differ!"
    exit 1
fi