# EXT2 File System Parser - Lab 13

**Student:** Fatima  
**Course:** Operating Systems  
**Lab:** 13 - EXT2 File System Internals  
**Date:** January 2026

## Overview

This program implements a low-level EXT2 file system parser that reads directly from a raw disk image without using the operating system's mounted file system services. It demonstrates understanding of how Linux stores and retrieves files at the disk block level.

## Purpose

The lab teaches:
- How the EXT2 file system works internally
- What inodes, blocks, and directory entries are
- How `ls` and `cp` work at the OS level
- Low-level disk I/O operations
- File system data structure parsing

## Implementation

### Features Implemented

✅ **EXT2 Image Loader**
- Opens raw disk image in binary mode
- Reads EXT2 structures directly from disk
- Validates superblock magic number
- Parses group descriptor table

✅ **ls Command (List Files)**
- Reads root directory inode
- Parses directory entries
- Displays file names, types, sizes, and inode numbers
- Shows complete directory listing

✅ **cp Command (Copy File)**
- Locates files by name in directory
- Reads file inode and metadata
- Follows block pointers (direct and indirect)
- Extracts file data from blocks
- Creates copy on host file system

✅ **info Command (Bonus)**
- Displays file system information
- Shows superblock details
- Lists group descriptor data
- Provides disk usage statistics

### Low-Level I/O Only

The implementation uses **only** low-level system calls:
- `open()` - Open raw disk image
- `read()` - Read bytes from disk
- `lseek()` - Seek to specific offsets
- `close()` - Close file descriptors

**NO use of:**
- `system("ls")`
- `system("cp")`
- Mounted directory access for reading files
- High-level C++ file streams for disk access

## EXT2 Parsing Details

### 1. Superblock Parsing

**Location:** Offset 1024 bytes from start of disk

**Key Information Extracted:**
- Magic number (0xEF53) for validation
- Block size calculation: `1024 << s_log_block_size`
- Total blocks and inodes
- Blocks/inodes per group
- Inode size (128 bytes for old revision, s_inode_size for new)

**Code:**
```cpp
readBytes(&superblock, sizeof(ext2_superblock), 1024);
block_size = 1024 << superblock.s_log_block_size;
```

### 2. Group Descriptor Table

**Location:** Block after superblock (usually block 2)

**Information Retrieved:**
- Block bitmap location
- Inode bitmap location  
- Inode table starting block
- Free blocks/inodes count

**Code:**
```cpp
uint32_t gdt_block = superblock.s_first_data_block + 1;
off_t gdt_offset = gdt_block * block_size;
readBytes(&group_desc, sizeof(ext2_group_desc), gdt_offset);
```

### 3. Inode Reading

**Calculation:**
```
inode_index = inode_number - 1  (inodes start at 1)
group = inode_index / s_inodes_per_group
local_index = inode_index % s_inodes_per_group
inode_offset = inode_table_block * block_size + local_index * inode_size
```

**Structure Parsed:**
- File mode and type
- File size in bytes
- Block pointers (12 direct, 1 indirect, 1 double, 1 triple)
- Timestamps
- Owner information

### 4. Directory Entry Parsing

**Format:**
```
struct ext2_dir_entry {
    uint32_t inode;      // Inode number
    uint16_t rec_len;    // Entry length
    uint8_t name_len;    // Filename length
    uint8_t file_type;   // Type (file/dir/link)
    char name[255];      // Variable length name
}
```

**Parsing Process:**
1. Read directory inode data blocks
2. Walk through entries using `rec_len`
3. Stop when `inode == 0` or `rec_len == 0`
4. Extract filename (null-terminate at `name_len`)

### 5. Data Block Reading

**Direct Blocks (0-11):**
```cpp
for (int i = 0; i < 12; i++) {
    if (inode.i_block[i] != 0) {
        readBlock(inode.i_block[i], buffer);
        // Copy data to output
    }
}
```

**Indirect Block (12):**
```cpp
if (inode.i_block[12] != 0) {
    uint32_t indirect_block[256];
    readBlock(inode.i_block[12], indirect_block);
    
    for (uint32_t i = 0; i < 256; i++) {
        if (indirect_block[i] != 0) {
            readBlock(indirect_block[i], data_buffer);
        }
    }
}
```

## Compilation and Usage

### Prerequisites

- Linux system (tested on Ubuntu)
- C++ compiler (g++)
- sudo access for disk image setup

### Setup Disk Image

```bash
# Make setup script executable
chmod +x setup_disk.sh

# Run setup (creates and populates disk image)
./setup_disk.sh
```

This creates `my_partition.img` with sample files.

### Compile Program

```bash
# Using Makefile
make

# Or manually
g++ -std=c++11 -o myfs ext2_parser.cpp
```

### Usage Examples

**List root directory:**
```bash
./myfs my_partition.img ls
```

**Copy file from image to host:**
```bash
./myfs my_partition.img cp sample.txt
./myfs my_partition.img cp hello.txt output.txt
```

**Show file system info:**
```bash
./myfs my_partition.img info
```

## Sample Output

### ls Command
```
========================================
DIRECTORY LISTING (Inode 2)
========================================
Name                          Type      Inode     Size
----------------------------------------
.                             DIR       2         1024 bytes
..                            DIR       2         1024 bytes
lost+found                    DIR       11        12288 bytes
sample.txt                    FILE      12        245 bytes
hello.txt                     FILE      13        18 bytes
data.txt                      FILE      14        156 bytes
hello.c                       FILE      15        78 bytes
test_dir                      DIR       16        1024 bytes
----------------------------------------
Total entries: 8
========================================
```

### cp Command
```
Successfully copied: sample.txt -> sample.txt
Size: 245 bytes
```

### info Command
```
========================================
EXT2 FILE SYSTEM INFORMATION
========================================
Magic Number: 0xef53
Block Size: 1024 bytes
Inode Size: 128 bytes
Total Blocks: 204800
Free Blocks: 199456
Total Inodes: 51200
Free Inodes: 51189
Blocks Per Group: 8192
Inodes Per Group: 51200
First Data Block: 1
...
========================================
```

## Architecture

### Disk Layout
```
┌─────────────────────────────────────┐
│ Block 0: Boot Block (Reserved)      │
├─────────────────────────────────────┤
│ Block 1: Superblock                 │
│   - Magic: 0xEF53                   │
│   - Block size, inode count, etc.   │
├─────────────────────────────────────┤
│ Block 2: Group Descriptor Table     │
│   - Bitmap locations                │
│   - Inode table location            │
├─────────────────────────────────────┤
│ Block N: Block Bitmap               │
├─────────────────────────────────────┤
│ Block N+1: Inode Bitmap             │
├─────────────────────────────────────┤
│ Block N+2: Inode Table              │
│   - Inode 2: Root directory         │
│   - Inode 11: lost+found            │
│   - Inode 12+: User files           │
├─────────────────────────────────────┤
│ Remaining Blocks: Data Blocks       │
│   - File contents                   │
│   - Directory data                  │
└─────────────────────────────────────┘
```

### File Read Workflow
```
1. User: Read "sample.txt"
   ↓
2. Read root directory (inode 2)
   ↓
3. Parse directory entries
   ↓
4. Find "sample.txt" → inode 12
   ↓
5. Read inode 12
   ↓
6. Get block pointers: [100, 0, 0, ...]
   ↓
7. Read block 100
   ↓
8. Extract file data
   ↓
9. Write to host file
```

## Supported Features

### ✅ Implemented
- Read superblock and validate
- Read group descriptor table
- Read inodes by number
- Parse directory entries
- Read file data (direct blocks)
- Read file data (indirect blocks)
- List directory contents
- Copy files from image to host
- Display file system information

### ⚠️ Limitations
- Only reads from root directory (no subdirectory navigation in cp)
- Supports up to indirect blocks (not double/triple indirect)
- Single block group support (works for images < 8 GB)
- Read-only operations (no write support)
- No symbolic link resolution

### 🔮 Potential Enhancements
- Recursive directory listing
- Path-based file access (/dir/subdir/file.txt)
- Double and triple indirect block support
- Multiple block group support
- File copying into image
- Directory creation
- File deletion
- Symbolic link following

## Technical Details

### Data Structure Sizes
- Superblock: 1024 bytes
- Group Descriptor: 32 bytes
- Inode: 128 bytes (or s_inode_size)
- Directory Entry: Variable (8 + name_len)

### Block Size
- Typical: 1024 bytes (1 KB)
- Calculated: `1024 << s_log_block_size`
- Can be 1024, 2048, or 4096 bytes

### Inode Numbers
- 0: Invalid (not used)
- 1: Bad blocks inode
- 2: Root directory
- 11: lost+found directory
- 12+: User files and directories

### File Types
- 0: Unknown
- 1: Regular file
- 2: Directory
- 3: Character device
- 4: Block device
- 5: FIFO
- 6: Socket
- 7: Symbolic link

## Testing

### Test Cases

**Test 1: List directory**
```bash
./myfs my_partition.img ls
# Verify all files appear
```

**Test 2: Copy text file**
```bash
./myfs my_partition.img cp sample.txt
cat sample.txt  # Verify content matches
```

**Test 3: Copy multiple files**
```bash
./myfs my_partition.img cp hello.txt
./myfs my_partition.img cp data.txt
./myfs my_partition.img cp hello.c
# Verify all files copied correctly
```

**Test 4: File system info**
```bash
./myfs my_partition.img info
# Verify block size, inode count, etc.
```

**Test 5: Non-existent file**
```bash
./myfs my_partition.img cp nonexistent.txt
# Should show error: File not found
```

## Challenges Overcome

### 1. Superblock Location
**Challenge:** Finding the correct offset for superblock  
**Solution:** Superblock always at offset 1024 bytes (not block 1)

### 2. Inode Calculation
**Challenge:** Converting inode number to disk offset  
**Solution:** Used formula: `inode_table + (local_index * inode_size)`

### 3. Directory Entry Parsing
**Challenge:** Variable-length entries with padding  
**Solution:** Used `rec_len` to skip to next entry

### 4. Block Size Calculation
**Challenge:** Block size stored as logarithm  
**Solution:** `block_size = 1024 << s_log_block_size`

### 5. Indirect Block Handling
**Challenge:** Files larger than 12 blocks need indirect pointers  
**Solution:** Read indirect block, iterate through pointers

## Reference Documentation

Primary reference: https://www.nongnu.org/ext2-doc/ext2.html

Key sections studied:
- Chapter 3: Disk Organization
- Chapter 4: Superblock
- Chapter 5: Block Group Descriptor Table
- Chapter 6: Inode Table
- Chapter 7: Directory Structure

## Submission Contents

1. **ext2_parser.cpp** - Main source code with full implementation
2. **README.md** - This documentation
3. **Makefile** - Build system
4. **setup_disk.sh** - Disk image creation script
5. **Screenshots** - Output examples (if required)

## Code Statistics

- **Total Lines:** ~800 lines
- **Comments:** ~200 lines
- **Structures:** 4 EXT2 data structures
- **Functions:** 15+ member functions
- **Low-level I/O calls:** read(), lseek(), open(), close()

## Learning Outcomes

Through this lab, I learned:

1. **File System Internals**
   - How EXT2 organizes data on disk
   - Block-based storage architecture
   - Inode structure and purpose

2. **Data Structures**
   - Superblock layout and fields
   - Group descriptors
   - Directory entries
   - Inode block pointers

3. **Low-Level Programming**
   - Binary file I/O
   - Structure packing and alignment
   - Offset calculations
   - Direct disk access

4. **OS Concepts**
   - How `ls` retrieves file listings
   - How `cp` extracts file data
   - Block allocation and tracking
   - Directory hierarchy

5. **Debugging Skills**
   - Using hexdump to verify structures
   - Calculating correct offsets
   - Validating magic numbers
   - Tracing block pointers

## Conclusion

This implementation successfully demonstrates understanding of the EXT2 file system internals by parsing raw disk structures and implementing basic file operations without using OS services. The code is well-documented, handles edge cases, and provides clear output for verification.

---

**Author:** Fatima  
**Date:** January 2026  
**Course:** Operating Systems - Lab 13
