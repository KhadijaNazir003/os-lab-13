#!/bin/bash

# EXT2 Parser Demonstration Script
# Lab 13 - Operating Systems

echo "========================================"
echo "  EXT2 PARSER DEMONSTRATION"
echo "  Lab 13 - Operating Systems"
echo "========================================"
echo ""
echo "Student: Fatima"
echo "Date: January 2026"
echo ""

# Check if parser is built
if [ ! -f "./myfs" ]; then
    echo "Building EXT2 parser..."
    make -f Makefile_lab13
    echo ""
fi

# Check if test image exists
if [ ! -f "test_partition.img" ]; then
    echo "Creating test disk image..."
    dd if=/dev/zero of=test_partition.img bs=1M count=10 2>&1 | tail -1
    mkfs.ext2 -F test_partition.img 2>&1 | grep "Creating"
    echo ""
fi

IMAGE="test_partition.img"

echo "========================================" 
echo "Demo 1: Show File System Information"
echo "========================================"
./myfs $IMAGE info

echo ""
echo "========================================"
echo "Demo 2: List Root Directory"
echo "========================================"
./myfs $IMAGE ls

echo ""
echo "========================================"
echo "Demo 3: Feature Highlights"
echo "========================================"
echo ""
echo "✓ Direct disk access (no mounting)"
echo "✓ EXT2 superblock parsing"
echo "✓ Group descriptor reading"
echo "✓ Inode table traversal"
echo "✓ Directory entry parsing"
echo "✓ Block pointer following"
echo ""

echo "========================================"
echo "Technical Details"
echo "========================================"
echo ""
echo "Implementation:"
echo "  - Low-level I/O only (open, read, lseek)"
echo "  - No system() calls"
echo "  - Direct structure parsing"
echo "  - ~800 lines of C++ code"
echo ""
echo "Supported:"
echo "  - Superblock validation (magic 0xEF53)"
echo "  - Direct block reading (blocks 0-11)"
echo "  - Indirect block reading (block 12)"
echo "  - Directory listing with metadata"
echo "  - File extraction and copying"
echo ""

echo "========================================"
echo "Demonstration Complete!"
echo "========================================"
echo ""
echo "Try these commands yourself:"
echo "  ./myfs $IMAGE info  - Show filesystem info"
echo "  ./myfs $IMAGE ls    - List root directory"
echo ""
echo "With a properly populated image:"
echo "  ./myfs my_partition.img cp sample.txt"
echo ""
