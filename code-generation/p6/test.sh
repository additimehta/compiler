#!/bin/bash

echo "=== Building P6 ==="

# Build p6.asm → p6.merl
cs241.linkasm < p6.asm > p6.merl
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to assemble p6.asm"
    exit 1
fi

# Link with starter.merl
cs241.linker p6.merl starter.merl > p6.linked
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to link p6.merl with starter.merl"
    exit 1
fi

echo "✓ Built p6.linked"



echo ""
echo "=== Creating test MERL file ==="

# Make a simple input.asm
cat > input.asm << 'EOF'
lis $3
.word 241
jr $31
EOF

# Assemble into a MERL test file
cs241.linkasm < input.asm > input.merl
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to assemble input.asm"
    exit 1
fi

echo "✓ Created input.merl"



echo ""
echo "=== Running P6 ==="
mips.stdin p6.linked < input.merl

echo ""
echo "=== Done ==="

