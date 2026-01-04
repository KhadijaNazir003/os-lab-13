/*
================================================================================
EXT2 FILE SYSTEM PARSER - LAB 13
================================================================================

PURPOSE:
This program parses an EXT2 file system from a raw disk image and implements
basic file system utilities (ls and cp) without using the operating system's
mounted file system services.

DESIGN APPROACH:
1. Read EXT2 structures directly from the raw disk image
2. Parse superblock to understand file system layout
3. Read group descriptor table to locate inode table
4. Implement inode reading and data block traversal
5. Parse directory entries to list files
6. Extract file data by following inode block pointers

EXT2 STRUCTURE OVERVIEW:
------------------------
Block 0: Boot Block (reserved)
Block 1: Superblock (file system metadata)
Block 2: Group Descriptor Table
Block N: Block Bitmap
Block N+1: Inode Bitmap  
Block N+2: Inode Table
Remaining: Data Blocks

KEY DATA STRUCTURES:
- Superblock: Contains file system parameters
- Group Descriptor: Points to bitmaps and inode table
- Inode: Contains file metadata and block pointers
- Directory Entry: Maps filenames to inode numbers

INODE STRUCTURE:
- 12 direct block pointers
- 1 indirect block pointer
- 1 double indirect block pointer
- 1 triple indirect block pointer

DIRECTORY ENTRY FORMAT:
- inode: Inode number
- rec_len: Record length
- name_len: Filename length
- file_type: File type indicator
- name: Filename (variable length)

IMPLEMENTATION NOTES:
- Uses only low-level I/O (open, read, lseek)
- No system() calls or OS file system access
- Direct byte-level parsing of EXT2 structures
- Supports files up to 12 direct blocks + indirect blocks

REFERENCES:
https://www.nongnu.org/ext2-doc/ext2.html

Author: Fatima
Course: Operating Systems - Lab 13
Date: January 2026
================================================================================
*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iomanip>

using namespace std;

// ============================================================================
// EXT2 CONSTANTS
// ============================================================================

#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_ROOT_INO 2
#define EXT2_BLOCK_SIZE 1024  // Default block size

// File types
#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_CHRDEV   3
#define EXT2_FT_BLKDEV   4
#define EXT2_FT_FIFO     5
#define EXT2_FT_SOCK     6
#define EXT2_FT_SYMLINK  7

// Inode modes
#define EXT2_S_IFREG  0x8000  // Regular file
#define EXT2_S_IFDIR  0x4000  // Directory
#define EXT2_S_IFLNK  0xA000  // Symbolic link

// ============================================================================
// EXT2 DATA STRUCTURES
// ============================================================================

#pragma pack(push, 1)

// EXT2 Superblock structure (located at offset 1024)
struct ext2_superblock {
    uint32_t s_inodes_count;        // Total inodes
    uint32_t s_blocks_count;        // Total blocks
    uint32_t s_r_blocks_count;      // Reserved blocks
    uint32_t s_free_blocks_count;   // Free blocks
    uint32_t s_free_inodes_count;   // Free inodes
    uint32_t s_first_data_block;    // First data block
    uint32_t s_log_block_size;      // Block size (log2(block_size) - 10)
    uint32_t s_log_frag_size;       // Fragment size
    uint32_t s_blocks_per_group;    // Blocks per group
    uint32_t s_frags_per_group;     // Fragments per group
    uint32_t s_inodes_per_group;    // Inodes per group
    uint32_t s_mtime;               // Mount time
    uint32_t s_wtime;               // Write time
    uint16_t s_mnt_count;           // Mount count
    uint16_t s_max_mnt_count;       // Max mount count
    uint16_t s_magic;               // Magic signature (0xEF53)
    uint16_t s_state;               // File system state
    uint16_t s_errors;              // Error handling
    uint16_t s_minor_rev_level;     // Minor revision
    uint32_t s_lastcheck;           // Last check time
    uint32_t s_checkinterval;       // Check interval
    uint32_t s_creator_os;          // Creator OS
    uint32_t s_rev_level;           // Revision level
    uint16_t s_def_resuid;          // Default reserved user ID
    uint16_t s_def_resgid;          // Default reserved group ID
    // EXT2_DYNAMIC_REV specific
    uint32_t s_first_ino;           // First non-reserved inode
    uint16_t s_inode_size;          // Inode size
    uint16_t s_block_group_nr;      // Block group number
    uint32_t s_feature_compat;      // Compatible features
    uint32_t s_feature_incompat;    // Incompatible features
    uint32_t s_feature_ro_compat;   // Read-only compatible features
    uint8_t  s_uuid[16];            // Volume UUID
    char     s_volume_name[16];     // Volume name
    char     s_last_mounted[64];    // Last mounted path
    uint32_t s_algorithm_usage_bitmap; // Compression algorithm
    uint8_t  s_prealloc_blocks;     // Blocks to preallocate
    uint8_t  s_prealloc_dir_blocks; // Directory blocks to preallocate
    uint16_t s_padding1;
    uint8_t  s_reserved[204];       // Padding to 1024 bytes
};

// Group Descriptor structure
struct ext2_group_desc {
    uint32_t bg_block_bitmap;       // Block bitmap block
    uint32_t bg_inode_bitmap;       // Inode bitmap block
    uint32_t bg_inode_table;        // Inode table block
    uint16_t bg_free_blocks_count;  // Free blocks count
    uint16_t bg_free_inodes_count;  // Free inodes count
    uint16_t bg_used_dirs_count;    // Directories count
    uint16_t bg_pad;
    uint8_t  bg_reserved[12];
};

// Inode structure
struct ext2_inode {
    uint16_t i_mode;        // File mode
    uint16_t i_uid;         // Owner UID
    uint32_t i_size;        // Size in bytes
    uint32_t i_atime;       // Access time
    uint32_t i_ctime;       // Creation time
    uint32_t i_mtime;       // Modification time
    uint32_t i_dtime;       // Deletion time
    uint16_t i_gid;         // Group ID
    uint16_t i_links_count; // Links count
    uint32_t i_blocks;      // Blocks count
    uint32_t i_flags;       // File flags
    uint32_t i_osd1;        // OS dependent
    uint32_t i_block[15];   // Pointers to blocks (12 direct, 1 indirect, 1 double, 1 triple)
    uint32_t i_generation;  // File version
    uint32_t i_file_acl;    // File ACL
    uint32_t i_dir_acl;     // Directory ACL
    uint32_t i_faddr;       // Fragment address
    uint8_t  i_osd2[12];    // OS dependent
};

// Directory entry structure
struct ext2_dir_entry {
    uint32_t inode;         // Inode number
    uint16_t rec_len;       // Record length
    uint8_t  name_len;      // Name length
    uint8_t  file_type;     // File type
    char     name[255];     // Filename (variable length)
};

#pragma pack(pop)

// ============================================================================
// EXT2 FILE SYSTEM CLASS
// ============================================================================

class EXT2Parser {
private:
    int fd;                         // File descriptor for image
    ext2_superblock superblock;     // Superblock
    ext2_group_desc group_desc;     // Group descriptor
    uint32_t block_size;            // Block size in bytes
    uint32_t inode_size;            // Inode size in bytes
    
    // ========================================================================
    // LOW-LEVEL I/O FUNCTIONS
    // ========================================================================
    
    // Read bytes from disk at specified offset
    ssize_t readBytes(void* buffer, size_t size, off_t offset) {
        if (lseek(fd, offset, SEEK_SET) == -1) {
            cerr << "Error: Failed to seek to offset " << offset << endl;
            return -1;
        }
        
        ssize_t bytes_read = read(fd, buffer, size);
        if (bytes_read < 0) {
            cerr << "Error: Failed to read " << size << " bytes at offset " << offset << endl;
            return -1;
        }
        
        return bytes_read;
    }
    
    // Read a complete block
    bool readBlock(uint32_t block_num, void* buffer) {
        off_t offset = block_num * block_size;
        ssize_t result = readBytes(buffer, block_size, offset);
        return result == (ssize_t)block_size;
    }
    
    // ========================================================================
    // EXT2 STRUCTURE PARSING
    // ========================================================================
    
    // Read and validate superblock
    bool readSuperblock() {
        // Superblock is at offset 1024
        if (readBytes(&superblock, sizeof(ext2_superblock), 1024) < 0) {
            cerr << "Error: Failed to read superblock" << endl;
            return false;
        }
        
        // Validate magic number
        if (superblock.s_magic != EXT2_SUPER_MAGIC) {
            cerr << "Error: Invalid EXT2 magic number: 0x" 
                 << hex << superblock.s_magic << dec << endl;
            cerr << "Expected: 0xEF53" << endl;
            return false;
        }
        
        // Calculate block size
        block_size = 1024 << superblock.s_log_block_size;
        
        // Get inode size
        if (superblock.s_rev_level == 0) {
            inode_size = 128;  // Old revision
        } else {
            inode_size = superblock.s_inode_size;
        }
        
        return true;
    }
    
    // Read group descriptor table
    bool readGroupDescriptor() {
        // Group descriptor table starts right after superblock
        uint32_t gdt_block = superblock.s_first_data_block + 1;
        off_t gdt_offset = gdt_block * block_size;
        
        if (readBytes(&group_desc, sizeof(ext2_group_desc), gdt_offset) < 0) {
            cerr << "Error: Failed to read group descriptor" << endl;
            return false;
        }
        
        return true;
    }
    
    // Read an inode by inode number
    bool readInode(uint32_t inode_num, ext2_inode& inode) {
        if (inode_num == 0 || inode_num > superblock.s_inodes_count) {
            cerr << "Error: Invalid inode number: " << inode_num << endl;
            return false;
        }
        
        // Calculate inode location
        // Inodes are numbered starting from 1
        uint32_t inode_index = inode_num - 1;
        uint32_t group = inode_index / superblock.s_inodes_per_group;
        uint32_t local_index = inode_index % superblock.s_inodes_per_group;
        
        // For simplicity, we use group 0 (works for small file systems)
        off_t inode_table_offset = group_desc.bg_inode_table * block_size;
        off_t inode_offset = inode_table_offset + (local_index * inode_size);
        
        if (readBytes(&inode, sizeof(ext2_inode), inode_offset) < 0) {
            cerr << "Error: Failed to read inode " << inode_num << endl;
            return false;
        }
        
        return true;
    }
    
    // ========================================================================
    // DATA BLOCK READING
    // ========================================================================
    
    // Read data from inode (handles direct and indirect blocks)
    bool readInodeData(const ext2_inode& inode, vector<uint8_t>& data) {
        data.clear();
        data.reserve(inode.i_size);
        
        uint8_t* block_buffer = new uint8_t[block_size];
        uint32_t blocks_needed = (inode.i_size + block_size - 1) / block_size;
        uint32_t bytes_read = 0;
        
        // Read direct blocks (first 12 blocks)
        for (int i = 0; i < 12 && i < (int)blocks_needed && bytes_read < inode.i_size; i++) {
            if (inode.i_block[i] == 0) break;
            
            if (!readBlock(inode.i_block[i], block_buffer)) {
                delete[] block_buffer;
                return false;
            }
            
            uint32_t to_copy = min(block_size, inode.i_size - bytes_read);
            data.insert(data.end(), block_buffer, block_buffer + to_copy);
            bytes_read += to_copy;
        }
        
        // Handle indirect blocks if needed
        if (bytes_read < inode.i_size && inode.i_block[12] != 0) {
            uint32_t* indirect_block = new uint32_t[block_size / sizeof(uint32_t)];
            
            if (readBlock(inode.i_block[12], indirect_block)) {
                uint32_t entries = block_size / sizeof(uint32_t);
                
                for (uint32_t i = 0; i < entries && bytes_read < inode.i_size; i++) {
                    if (indirect_block[i] == 0) break;
                    
                    if (!readBlock(indirect_block[i], block_buffer)) {
                        delete[] indirect_block;
                        delete[] block_buffer;
                        return false;
                    }
                    
                    uint32_t to_copy = min(block_size, inode.i_size - bytes_read);
                    data.insert(data.end(), block_buffer, block_buffer + to_copy);
                    bytes_read += to_copy;
                }
            }
            
            delete[] indirect_block;
        }
        
        delete[] block_buffer;
        return true;
    }
    
    // ========================================================================
    // DIRECTORY PARSING
    // ========================================================================
    
    // Parse directory entries from directory data
    void parseDirectoryEntries(const vector<uint8_t>& data, 
                              vector<ext2_dir_entry>& entries) {
        entries.clear();
        size_t offset = 0;
        
        while (offset < data.size()) {
            ext2_dir_entry* entry = (ext2_dir_entry*)(data.data() + offset);
            
            if (entry->inode == 0 || entry->rec_len == 0) {
                break;
            }
            
            // Copy entry (only copy the actual name length)
            ext2_dir_entry dir_entry;
            memcpy(&dir_entry, entry, sizeof(ext2_dir_entry) - 255);
            memcpy(dir_entry.name, entry->name, entry->name_len);
            dir_entry.name[entry->name_len] = '\0';  // Null terminate
            
            entries.push_back(dir_entry);
            
            offset += entry->rec_len;
        }
    }
    
    // Find a file in directory by name
    bool findFileInDirectory(uint32_t dir_inode_num, const string& filename, 
                            uint32_t& found_inode) {
        ext2_inode dir_inode;
        if (!readInode(dir_inode_num, dir_inode)) {
            return false;
        }
        
        // Check if it's a directory
        if ((dir_inode.i_mode & 0xF000) != EXT2_S_IFDIR) {
            cerr << "Error: Inode " << dir_inode_num << " is not a directory" << endl;
            return false;
        }
        
        // Read directory data
        vector<uint8_t> dir_data;
        if (!readInodeData(dir_inode, dir_data)) {
            return false;
        }
        
        // Parse directory entries
        vector<ext2_dir_entry> entries;
        parseDirectoryEntries(dir_data, entries);
        
        // Search for file
        for (const auto& entry : entries) {
            if (strcmp(entry.name, filename.c_str()) == 0) {
                found_inode = entry.inode;
                return true;
            }
        }
        
        return false;
    }
    
public:
    // ========================================================================
    // CONSTRUCTOR & INITIALIZATION
    // ========================================================================
    
    EXT2Parser() : fd(-1), block_size(1024), inode_size(128) {}
    
    ~EXT2Parser() {
        if (fd >= 0) {
            close(fd);
        }
    }
    
    // Open and initialize EXT2 image
    bool open(const string& image_path) {
        // Open image file in read-only mode
        fd = ::open(image_path.c_str(), O_RDONLY);
        if (fd < 0) {
            cerr << "Error: Cannot open image file: " << image_path << endl;
            return false;
        }
        
        // Read superblock
        if (!readSuperblock()) {
            close(fd);
            fd = -1;
            return false;
        }
        
        // Read group descriptor
        if (!readGroupDescriptor()) {
            close(fd);
            fd = -1;
            return false;
        }
        
        return true;
    }
    
    // ========================================================================
    // FILE SYSTEM OPERATIONS
    // ========================================================================
    
    // List directory contents (ls command)
    void listDirectory(uint32_t dir_inode_num = EXT2_ROOT_INO) {
        ext2_inode inode;
        if (!readInode(dir_inode_num, inode)) {
            cerr << "Error: Failed to read directory inode" << endl;
            return;
        }
        
        // Check if it's a directory
        if ((inode.i_mode & 0xF000) != EXT2_S_IFDIR) {
            cerr << "Error: Inode is not a directory" << endl;
            return;
        }
        
        // Read directory data
        vector<uint8_t> dir_data;
        if (!readInodeData(inode, dir_data)) {
            cerr << "Error: Failed to read directory data" << endl;
            return;
        }
        
        // Parse directory entries
        vector<ext2_dir_entry> entries;
        parseDirectoryEntries(dir_data, entries);
        
        // Display entries
        cout << "\n========================================" << endl;
        cout << "DIRECTORY LISTING (Inode " << dir_inode_num << ")" << endl;
        cout << "========================================" << endl;
        cout << left << setw(30) << "Name" 
             << setw(10) << "Type" 
             << setw(10) << "Inode"
             << "Size" << endl;
        cout << "----------------------------------------" << endl;
        
        for (const auto& entry : entries) {
            // Skip . and .. for cleaner output (optional)
            // if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) continue;
            
            // Get inode to check size
            ext2_inode file_inode;
            string type_str = "UNKNOWN";
            uint32_t size = 0;
            
            if (readInode(entry.inode, file_inode)) {
                size = file_inode.i_size;
                
                switch (entry.file_type) {
                    case EXT2_FT_REG_FILE: type_str = "FILE"; break;
                    case EXT2_FT_DIR:      type_str = "DIR"; break;
                    case EXT2_FT_SYMLINK:  type_str = "LINK"; break;
                    case EXT2_FT_CHRDEV:   type_str = "CHR"; break;
                    case EXT2_FT_BLKDEV:   type_str = "BLK"; break;
                    case EXT2_FT_FIFO:     type_str = "FIFO"; break;
                    case EXT2_FT_SOCK:     type_str = "SOCK"; break;
                }
            }
            
            cout << left << setw(30) << entry.name
                 << setw(10) << type_str
                 << setw(10) << entry.inode
                 << size << " bytes" << endl;
        }
        
        cout << "----------------------------------------" << endl;
        cout << "Total entries: " << entries.size() << endl;
        cout << "========================================\n" << endl;
    }
    
    // Copy file from image to host (cp command)
    bool copyFileOut(const string& filename, const string& dest_path = "") {
        // Find file in root directory
        uint32_t file_inode_num;
        if (!findFileInDirectory(EXT2_ROOT_INO, filename, file_inode_num)) {
            cerr << "Error: File not found: " << filename << endl;
            return false;
        }
        
        // Read file inode
        ext2_inode file_inode;
        if (!readInode(file_inode_num, file_inode)) {
            cerr << "Error: Failed to read file inode" << endl;
            return false;
        }
        
        // Check if it's a regular file
        if ((file_inode.i_mode & 0xF000) != EXT2_S_IFREG) {
            cerr << "Error: Not a regular file" << endl;
            return false;
        }
        
        // Read file data
        vector<uint8_t> file_data;
        if (!readInodeData(file_inode, file_data)) {
            cerr << "Error: Failed to read file data" << endl;
            return false;
        }
        
        // Determine output path
        string output_path = dest_path.empty() ? filename : dest_path;
        
        // Write to host file system
        int out_fd = ::open(output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd < 0) {
            cerr << "Error: Cannot create output file: " << output_path << endl;
            return false;
        }
        
        ssize_t written = write(out_fd, file_data.data(), file_data.size());
        close(out_fd);
        
        if (written != (ssize_t)file_data.size()) {
            cerr << "Error: Failed to write complete file" << endl;
            return false;
        }
        
        cout << "Successfully copied: " << filename << " -> " << output_path << endl;
        cout << "Size: " << file_data.size() << " bytes" << endl;
        
        return true;
    }
    
    // Show file system information
    void showInfo() {
        cout << "\n========================================" << endl;
        cout << "EXT2 FILE SYSTEM INFORMATION" << endl;
        cout << "========================================" << endl;
        cout << "Magic Number: 0x" << hex << superblock.s_magic << dec << endl;
        cout << "Block Size: " << block_size << " bytes" << endl;
        cout << "Inode Size: " << inode_size << " bytes" << endl;
        cout << "Total Blocks: " << superblock.s_blocks_count << endl;
        cout << "Free Blocks: " << superblock.s_free_blocks_count << endl;
        cout << "Total Inodes: " << superblock.s_inodes_count << endl;
        cout << "Free Inodes: " << superblock.s_free_inodes_count << endl;
        cout << "Blocks Per Group: " << superblock.s_blocks_per_group << endl;
        cout << "Inodes Per Group: " << superblock.s_inodes_per_group << endl;
        cout << "First Data Block: " << superblock.s_first_data_block << endl;
        
        if (superblock.s_volume_name[0] != '\0') {
            cout << "Volume Name: " << superblock.s_volume_name << endl;
        }
        
        cout << "\nGroup Descriptor (Group 0):" << endl;
        cout << "Block Bitmap: Block " << group_desc.bg_block_bitmap << endl;
        cout << "Inode Bitmap: Block " << group_desc.bg_inode_bitmap << endl;
        cout << "Inode Table: Block " << group_desc.bg_inode_table << endl;
        cout << "Free Blocks: " << group_desc.bg_free_blocks_count << endl;
        cout << "Free Inodes: " << group_desc.bg_free_inodes_count << endl;
        cout << "Used Directories: " << group_desc.bg_used_dirs_count << endl;
        cout << "========================================\n" << endl;
    }
};

// ============================================================================
// MAIN PROGRAM
// ============================================================================

void showUsage(const char* prog_name) {
    cout << "========================================" << endl;
    cout << "  EXT2 File System Parser - Lab 13" << endl;
    cout << "========================================" << endl;
    cout << "Usage:" << endl;
    cout << "  " << prog_name << " <image> ls           - List root directory" << endl;
    cout << "  " << prog_name << " <image> cp <file>    - Copy file from image to host" << endl;
    cout << "  " << prog_name << " <image> info         - Show file system info" << endl;
    cout << "\nExamples:" << endl;
    cout << "  " << prog_name << " my_partition.img ls" << endl;
    cout << "  " << prog_name << " my_partition.img cp test.txt" << endl;
    cout << "  " << prog_name << " my_partition.img info" << endl;
    cout << "========================================" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        showUsage(argv[0]);
        return 1;
    }
    
    string image_path = argv[1];
    string command = argv[2];
    
    // Initialize parser
    EXT2Parser parser;
    
    if (!parser.open(image_path)) {
        cerr << "Error: Failed to open EXT2 image: " << image_path << endl;
        return 1;
    }
    
    // Execute command
    if (command == "ls") {
        parser.listDirectory();
    }
    else if (command == "cp") {
        if (argc < 4) {
            cerr << "Error: cp command requires filename" << endl;
            cout << "Usage: " << argv[0] << " <image> cp <filename>" << endl;
            return 1;
        }
        
        string filename = argv[3];
        string dest_path = (argc >= 5) ? argv[4] : filename;
        
        if (!parser.copyFileOut(filename, dest_path)) {
            return 1;
        }
    }
    else if (command == "info") {
        parser.showInfo();
    }
    else {
        cerr << "Error: Unknown command: " << command << endl;
        showUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
