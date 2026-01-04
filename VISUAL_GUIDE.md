# EXT2 FILE SYSTEM STRUCTURES - VISUAL GUIDE
# Lab 13 - Operating Systems

## EXT2 DISK LAYOUT

```
┌──────────────────────────────────────────────────────────────┐
│                    PHYSICAL DISK IMAGE                        │
│                  (my_partition.img - 200 MB)                  │
└──────────────────────────────────────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────────┐
│  BLOCK 0 (0-1023 bytes)                                      │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  BOOT BLOCK (Reserved)                                 │  │
│  │  Not used by EXT2                                      │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  OFFSET 1024 (bytes 1024-2047)                               │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  SUPERBLOCK                                            │  │
│  │  ┌──────────────────────────────────────────────────┐  │  │
│  │  │  Magic: 0xEF53 (validation)                      │  │  │
│  │  │  Block Size: 1024 << s_log_block_size            │  │  │
│  │  │  Total Blocks: s_blocks_count                    │  │  │
│  │  │  Total Inodes: s_inodes_count                    │  │  │
│  │  │  Inodes Per Group: s_inodes_per_group            │  │  │
│  │  │  First Data Block: s_first_data_block            │  │  │
│  │  └──────────────────────────────────────────────────┘  │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCK (s_first_data_block + 1)                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  GROUP DESCRIPTOR TABLE                                │  │
│  │  ┌──────────────────────────────────────────────────┐  │  │
│  │  │  bg_block_bitmap: Block bitmap location          │  │  │
│  │  │  bg_inode_bitmap: Inode bitmap location          │  │  │
│  │  │  bg_inode_table: Inode table start block         │  │  │
│  │  │  bg_free_blocks_count: Free blocks               │  │  │
│  │  │  bg_free_inodes_count: Free inodes               │  │  │
│  │  └──────────────────────────────────────────────────┘  │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCK (bg_block_bitmap)                                     │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  BLOCK BITMAP                                          │  │
│  │  [1][1][1][1][0][0][0][1][1][0]...                     │  │
│  │   Used ↑           Free ↑                              │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCK (bg_inode_bitmap)                                     │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  INODE BITMAP                                          │  │
│  │  [1][1][1][1][0][0][1][0][1][0]...                     │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCKS (bg_inode_table .. bg_inode_table + N)               │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  INODE TABLE                                           │  │
│  │  ┌──────────────────────────────────────────────────┐  │  │
│  │  │  Inode 1: Bad blocks inode                       │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Inode 2: ROOT DIRECTORY                         │  │  │
│  │  │    i_mode: 0x41ED (directory, rwxr-xr-x)         │  │  │
│  │  │    i_size: 1024 bytes                            │  │  │
│  │  │    i_block[0]: 100 → Data block 100              │  │  │
│  │  │    i_block[1-14]: 0 (unused)                     │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Inode 11: lost+found directory                  │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Inode 12: sample.txt                            │  │  │
│  │  │    i_mode: 0x81A4 (file, rw-r--r--)              │  │  │
│  │  │    i_size: 245 bytes                             │  │  │
│  │  │    i_block[0]: 500 → Data block 500              │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  ...more inodes...                               │  │  │
│  │  └──────────────────────────────────────────────────┘  │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCK 100 (Root directory data)                             │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  DIRECTORY ENTRIES                                     │  │
│  │  ┌──────────────────────────────────────────────────┐  │  │
│  │  │  Entry 1:                                        │  │  │
│  │  │    inode: 2                                      │  │  │
│  │  │    rec_len: 12                                   │  │  │
│  │  │    name_len: 1                                   │  │  │
│  │  │    file_type: 2 (directory)                      │  │  │
│  │  │    name: "."                                     │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Entry 2:                                        │  │  │
│  │  │    inode: 2                                      │  │  │
│  │  │    rec_len: 12                                   │  │  │
│  │  │    name: ".."                                    │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Entry 3:                                        │  │  │
│  │  │    inode: 11                                     │  │  │
│  │  │    rec_len: 20                                   │  │  │
│  │  │    name: "lost+found"                            │  │  │
│  │  ├──────────────────────────────────────────────────┤  │  │
│  │  │  Entry 4:                                        │  │  │
│  │  │    inode: 12                                     │  │  │
│  │  │    rec_len: 20                                   │  │  │
│  │  │    name: "sample.txt"                            │  │  │
│  │  └──────────────────────────────────────────────────┘  │  │
│  └────────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────┤
│  BLOCK 500 (sample.txt data)                                 │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  FILE DATA                                             │  │
│  │  "This is a sample text file stored in the EXT2..."   │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
```

## INODE STRUCTURE DETAIL

```
ext2_inode (128 or 256 bytes)
┌────────────────────────────────────────────────────────────┐
│  Offset 0-1:   i_mode (file type and permissions)          │
│                15-12: Type (4=dir, 8=file, A=link)         │
│                11-0: Permissions (rwxrwxrwx)               │
├────────────────────────────────────────────────────────────┤
│  Offset 2-3:   i_uid (owner user ID)                       │
├────────────────────────────────────────────────────────────┤
│  Offset 4-7:   i_size (file size in bytes)                 │
├────────────────────────────────────────────────────────────┤
│  Offset 8-11:  i_atime (last access time)                  │
├────────────────────────────────────────────────────────────┤
│  Offset 12-15: i_ctime (creation time)                     │
├────────────────────────────────────────────────────────────┤
│  Offset 16-19: i_mtime (modification time)                 │
├────────────────────────────────────────────────────────────┤
│  Offset 20-23: i_dtime (deletion time)                     │
├────────────────────────────────────────────────────────────┤
│  Offset 24-25: i_gid (group ID)                            │
├────────────────────────────────────────────────────────────┤
│  Offset 26-27: i_links_count (hard links count)            │
├────────────────────────────────────────────────────────────┤
│  Offset 28-31: i_blocks (512-byte blocks count)            │
├────────────────────────────────────────────────────────────┤
│  Offset 32-35: i_flags (file flags)                        │
├────────────────────────────────────────────────────────────┤
│  Offset 40-43: i_block[0]  ─┐                              │
│  Offset 44-47: i_block[1]   │                              │
│  Offset 48-51: i_block[2]   │ Direct block pointers        │
│  ...            ...          │ (blocks 0-11)                │
│  Offset 84-87: i_block[11] ─┘                              │
├────────────────────────────────────────────────────────────┤
│  Offset 88-91: i_block[12] → Indirect block                │
│                               ┌─────────────────────────┐   │
│                               │ Block contains:         │   │
│                               │ [ptr][ptr][ptr]...[ptr] │   │
│                               │  ↓    ↓    ↓        ↓   │   │
│                               │ Data blocks 12-267      │   │
│                               └─────────────────────────┘   │
├────────────────────────────────────────────────────────────┤
│  Offset 92-95: i_block[13] → Double indirect block         │
├────────────────────────────────────────────────────────────┤
│  Offset 96-99: i_block[14] → Triple indirect block         │
└────────────────────────────────────────────────────────────┘
```

## DIRECTORY ENTRY STRUCTURE

```
ext2_dir_entry (variable size: 8 + name_len)
┌────────────────────────────────────────────────────────────┐
│  Offset 0-3:   inode (inode number)                        │
│                0 = entry is empty                          │
├────────────────────────────────────────────────────────────┤
│  Offset 4-5:   rec_len (entry length, includes padding)    │
│                Used to jump to next entry                  │
├────────────────────────────────────────────────────────────┤
│  Offset 6:     name_len (actual filename length)           │
├────────────────────────────────────────────────────────────┤
│  Offset 7:     file_type (0-7, see below)                  │
├────────────────────────────────────────────────────────────┤
│  Offset 8+:    name (variable length, NOT null-terminated) │
│                Use name_len to extract                     │
└────────────────────────────────────────────────────────────┘

File Types:
  0: Unknown
  1: Regular file
  2: Directory
  3: Character device
  4: Block device
  5: FIFO
  6: Socket
  7: Symbolic link

Example Entry:
  inode: 12
  rec_len: 20 (8 header + 10 name + 2 padding)
  name_len: 10
  file_type: 1
  name: "sample.txt" (10 chars, not null-terminated)
```

## FILE READ OPERATION FLOW

```
./myfs my_partition.img cp sample.txt
         │
         ▼
┌────────────────────────────────────────┐
│ 1. Open image file (binary mode)      │
│    fd = open("my_partition.img", O_RDONLY) │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 2. Read Superblock (offset 1024)      │
│    lseek(fd, 1024, SEEK_SET)           │
│    read(fd, &superblock, 1024)         │
│    Validate: magic == 0xEF53           │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 3. Read Group Descriptor               │
│    block = s_first_data_block + 1      │
│    offset = block * block_size         │
│    read(fd, &group_desc, 32)           │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 4. Read Root Directory Inode (inode 2) │
│    inode_table = group_desc.bg_inode_table │
│    offset = inode_table * block_size + │
│             (1 * inode_size)           │  (inode 2-1=1)
│    read(fd, &root_inode, inode_size)   │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 5. Read Root Directory Data            │
│    block_num = root_inode.i_block[0]   │
│    offset = block_num * block_size     │
│    read(fd, dir_data, block_size)      │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 6. Parse Directory Entries             │
│    Walk through entries using rec_len  │
│    Compare name with "sample.txt"      │
│    Found! inode = 12                   │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 7. Read File Inode (inode 12)          │
│    offset = inode_table * block_size + │
│             (11 * inode_size)          │  (inode 12-1=11)
│    read(fd, &file_inode, inode_size)   │
│    file_size = file_inode.i_size       │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 8. Read File Data Blocks               │
│    For i = 0 to 11:                    │
│      if i_block[i] != 0:               │
│        offset = i_block[i] * block_size│
│        read(fd, buffer, block_size)    │
│        append to file_data             │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 9. Handle Indirect Blocks (if needed)  │
│    if i_block[12] != 0:                │
│      Read indirect block               │
│      For each pointer in block:        │
│        Read data block                 │
└──────────────┬─────────────────────────┘
               ▼
┌────────────────────────────────────────┐
│ 10. Write to Host File                 │
│     out_fd = open("sample.txt",        │
│                   O_WRONLY|O_CREAT)    │
│     write(out_fd, file_data, file_size)│
│     close(out_fd)                      │
└────────────────────────────────────────┘
```

## BLOCK SIZE CALCULATION

```
From Superblock:
  s_log_block_size = 0, 1, 2, or 3

Calculation:
  block_size = 1024 << s_log_block_size

Examples:
  s_log_block_size = 0 → 1024 << 0 = 1024 bytes (1 KB)
  s_log_block_size = 1 → 1024 << 1 = 2048 bytes (2 KB)
  s_log_block_size = 2 → 1024 << 2 = 4096 bytes (4 KB)
  s_log_block_size = 3 → 1024 << 3 = 8192 bytes (8 KB)
```

## INODE NUMBER TO DISK OFFSET

```
Given: inode_number (e.g., 12 for sample.txt)

Calculate:
  inode_index = inode_number - 1          // Inodes start at 1
  group = inode_index / s_inodes_per_group
  local_index = inode_index % s_inodes_per_group
  
  inode_table_block = group_desc[group].bg_inode_table
  inode_offset = inode_table_block * block_size + 
                 local_index * inode_size

Example (inode 12, block_size=1024, inode_size=128):
  inode_index = 12 - 1 = 11
  group = 11 / 51200 = 0  (first group)
  local_index = 11 % 51200 = 11
  
  If bg_inode_table = 5:
    offset = 5 * 1024 + 11 * 128
    offset = 5120 + 1408 = 6528 bytes
```

## KEY VALIDATIONS

```
1. Superblock Magic Number
   ✓ Must be 0xEF53
   ✗ Anything else = not EXT2

2. Inode Numbers
   ✓ Must be > 0 and <= s_inodes_count
   ✗ 0 = invalid
   ✗ > s_inodes_count = out of range

3. Block Numbers
   ✓ Must be > 0 and < s_blocks_count
   ✗ 0 = unallocated
   ✗ >= s_blocks_count = invalid

4. File Types
   ✓ i_mode & 0xF000:
      0x4000 = directory
      0x8000 = regular file
      0xA000 = symbolic link
```

---

This visual guide provides a complete understanding of how EXT2
structures are laid out on disk and how the parser navigates them.
