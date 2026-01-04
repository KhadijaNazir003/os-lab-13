#!/bin/bash

# EXT2 Disk Image Setup Script
# Lab 13 - Operating Systems

echo "========================================"
echo "  EXT2 Disk Image Setup - Lab 13"
echo "========================================"
echo ""

IMAGE_NAME="my_partition.img"
MOUNT_POINT="/mnt/fs_assignment"

# Check if running as root for mount operations
if [ "$EUID" -ne 0 ]; then 
    echo "Note: Some operations require sudo privileges"
    echo "The script will prompt for password when needed."
    echo ""
fi

# Step 1: Create raw disk image
echo "Step 1: Creating 200 MB disk image..."
if [ -f "$IMAGE_NAME" ]; then
    read -p "Image already exists. Overwrite? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Setup cancelled."
        exit 1
    fi
fi

dd if=/dev/zero of=$IMAGE_NAME bs=1M count=200
if [ $? -ne 0 ]; then
    echo "Error: Failed to create disk image"
    exit 1
fi
echo "✓ Disk image created: $IMAGE_NAME"
echo ""

# Step 2: Format with EXT2
echo "Step 2: Formatting with EXT2 file system..."
mkfs.ext2 $IMAGE_NAME
if [ $? -ne 0 ]; then
    echo "Error: Failed to format disk image"
    exit 1
fi
echo "✓ EXT2 file system created"
echo ""

# Step 3: Mount the image
echo "Step 3: Mounting disk image..."
sudo mkdir -p $MOUNT_POINT
sudo mount -o loop $IMAGE_NAME $MOUNT_POINT
if [ $? -ne 0 ]; then
    echo "Error: Failed to mount disk image"
    exit 1
fi
echo "✓ Mounted at: $MOUNT_POINT"
echo ""

# Step 4: Add sample files
echo "Step 4: Adding sample files and directories..."

# Create sample text file
sudo bash -c "cat > $MOUNT_POINT/sample.txt << 'EOF'
This is a sample text file stored in the EXT2 file system.
It contains multiple lines of text for testing purposes.

EXT2 File System Lab - Operating Systems
Student: Fatima
Date: January 2026

This file will be used to test the ls and cp commands.
EOF"

# Create another file
sudo bash -c "echo 'Hello from EXT2!' > $MOUNT_POINT/hello.txt"

# Create a directory
sudo mkdir $MOUNT_POINT/test_dir

# Create file in subdirectory
sudo bash -c "echo 'File in subdirectory' > $MOUNT_POINT/test_dir/subfile.txt"

# Create a longer file
sudo bash -c "cat > $MOUNT_POINT/data.txt << 'EOF'
Line 1: Lorem ipsum dolor sit amet
Line 2: Consectetur adipiscing elit
Line 3: Sed do eiusmod tempor incididunt
Line 4: Ut labore et dolore magna aliqua
Line 5: End of data file
EOF"

# Create a code file
sudo bash -c "cat > $MOUNT_POINT/hello.c << 'EOF'
#include <stdio.h>

int main() {
    printf(\"Hello from EXT2!\\n\");
    return 0;
}
EOF"

echo "✓ Sample files created:"
echo "  - sample.txt (multi-line text)"
echo "  - hello.txt (simple text)"
echo "  - data.txt (data file)"
echo "  - hello.c (C source code)"
echo "  - test_dir/ (directory with subfile.txt)"
echo ""

# List files
echo "Step 5: Verifying files (using system ls)..."
sudo ls -lh $MOUNT_POINT/
echo ""

# Step 6: Unmount
echo "Step 6: Unmounting disk image..."
sudo umount $MOUNT_POINT
if [ $? -ne 0 ]; then
    echo "Error: Failed to unmount disk image"
    exit 1
fi
echo "✓ Disk image unmounted"
echo ""

echo "========================================"
echo "  Setup Complete!"
echo "========================================"
echo ""
echo "Disk image ready: $IMAGE_NAME"
echo ""
echo "Next steps:"
echo "  1. Compile the parser: make"
echo "  2. List files: ./myfs $IMAGE_NAME ls"
echo "  3. Copy file: ./myfs $IMAGE_NAME cp sample.txt"
echo ""
echo "Note: The disk image contains:"
echo "  - Root directory with sample files"
echo "  - test_dir subdirectory"
echo "  - Various file types for testing"
echo ""
echo "=========================================="
