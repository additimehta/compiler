#!/bin/bash
echo "=== Running tests for selected .wlp4 files ==="
echo ""

# Must provide at least one file
if [ $# -eq 0 ]; then
    echo "Usage: $0 file1.wlp4 [file2.wlp4 ...]"
    exit 1
fi

# Check for required libraries
ALLOC_MERL="alloc.merl"
PRINT_MERL="print.merl"

if [ ! -f "$ALLOC_MERL" ]; then
    echo "✗ ERROR: $ALLOC_MERL not found!"
    echo "  Download from: https://www.student.cs.uwaterloo.ca/~cs241/wlp4/alloc.merl"
    exit 1
fi

if [ ! -f "$PRINT_MERL" ]; then
    echo "⚠ WARNING: $PRINT_MERL not found!"
    echo "  Tests with println may fail"
fi

FILES=("$@")

for INPUT in "${FILES[@]}"; do
    if [ ! -f "$INPUT" ]; then
        echo "✗ ERROR: File not found: $INPUT"
        continue
    fi
    
    BASE=$(basename "$INPUT" .wlp4)
    
    echo "=============================================="
    echo "=== Testing: $INPUT ==="
    echo "=============================================="
    
    # Step 1: Parsing
    echo "Step 1: Parsing..."
    if ! wlp4scan < "$INPUT" | wlp4parse > "${BASE}.wlp4i" 2> "${BASE}.parse_err"; then
        echo "✗ Parsing failed!"
        cat "${BASE}.parse_err"
        continue
    fi
    echo "✓ Parsing passed"
    
    # Step 2: Type checking
    echo "Step 2: Type checking..."
    if ! wlp4type < "${BASE}.wlp4i" > "${BASE}.wlp4ti" 2> "${BASE}.type_err"; then
        echo "✗ Type checking crashed!"
        cat "${BASE}.type_err"
        continue
    fi
    if grep -q "ERROR" "${BASE}.wlp4ti"; then
        echo "✗ Type checking failed"
        continue
    fi
    echo "✓ Type checking passed"
    
    # Step 3: Compile code generator
    echo "Step 3: Compiling code generator..."
    if ! g++ -std=c++17 -Wall -O2 -o wlp4gen wlp4gen.cc; then
        echo "✗ Compilation failed!"
        exit 1
    fi
    echo "✓ Code generator compiled"
    
    # Step 4: Generate assembly
    echo "Step 4: Generating assembly..."
    if ! ./wlp4gen < "${BASE}.wlp4ti" > "${BASE}.asm" 2> "${BASE}.gen_err"; then
        echo "✗ Code generation failed!"
        cat "${BASE}.gen_err"
        continue
    fi
    echo "✓ Assembly generated"
    
    # Step 5: Link with alloc.merl (MUST BE LAST!)
    if grep -q "^\.import" "${BASE}.asm"; then
        echo "Step 5: Using linkasm and linking with alloc.merl..."
        
        if ! cs241.linkasm < "${BASE}.asm" > "${BASE}.merl"; then
            echo "✗ linkasm failed!"
            continue
        fi
        
        # Link with print.merl AND alloc.merl (alloc MUST be last!)
        if [ -f "$PRINT_MERL" ]; then
            if ! cs241.linker "${BASE}.merl" "$PRINT_MERL" "$ALLOC_MERL" > "${BASE}.linked.merl"; then
                echo "✗ linker failed!"
                continue
            fi
        else
            if ! cs241.linker "${BASE}.merl" "$ALLOC_MERL" > "${BASE}.linked.merl"; then
                echo "✗ linker failed!"
                continue
            fi
        fi
        
        # Strip MERL metadata (CRITICAL for alloc to work!)
        echo "  Stripping MERL metadata..."
        if ! cs241.merl 0 < "${BASE}.linked.merl" > "${BASE}.mips" 2> /dev/null; then
            echo "✗ MERL stripping failed!"
            continue
        fi
        
        ACTUAL_FILE="${BASE}.mips"
    else
        echo "Step 5: Using binasm..."
        if ! cs241.binasm < "${BASE}.asm" > "${BASE}.mips"; then
            echo "✗ Assembly failed!"
            continue
        fi
        ACTUAL_FILE="${BASE}.mips"
    fi
    
    # Step 6: Expected output
    echo "Step 6: Generating expected output..."
    if ! wlp4c < "$INPUT" > "${BASE}-expected.mips"; then
        echo "✗ wlp4c failed!"
        continue
    fi
    
    # Step 7: Run mips.array
    echo ""
    echo "=== Step 7: Comparing behavior with array input ==="
    echo ""
    ARRAY_INPUT="5 0 2 4 6 8"
    echo "Using input:"
    echo "$ARRAY_INPUT"
    echo ""
    
    ACTUAL_OUT=$(echo "$ARRAY_INPUT" | mips.array "$ACTUAL_FILE" 2>&1)
    EXPECT_OUT=$(echo "$ARRAY_INPUT" | mips.array "${BASE}-expected.mips" 2>&1)
    
    echo "Your output:"
    echo "$ACTUAL_OUT"
    echo ""
    echo "Expected:"
    echo "$EXPECT_OUT"
    echo ""
    
    ACTUAL_REG=$(echo "$ACTUAL_OUT" | grep '\$03 = ' | sed 's/.*\$03 = \([^ ]*\).*/\1/')
    EXPECT_REG=$(echo "$EXPECT_OUT" | grep '\$03 = ' | sed 's/.*\$03 = \([^ ]*\).*/\1/')
    
    if [ "$ACTUAL_REG" == "$EXPECT_REG" ]; then
        echo "✓ PASS: Return values match."
    else
        echo "✗ FAIL: Return values differ"
        echo "  Yours:    $ACTUAL_REG"
        echo "  Expected: $EXPECT_REG"
    fi
    echo ""
done

echo "=== SELECTED TESTING COMPLETE ==="