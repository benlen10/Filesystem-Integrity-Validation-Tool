// Author: Benjamin Lenington
// UW Email: lenington@wisc.edu
// Course: CS537 - Intro to Operating Systems
// Project: P5A - File System Checker

// Type definitions
#define DIR_TYPE 1
#define FILE_TYPE 2
#define DEV_TYPE 3

// Constant definitions
#define ROOT_NODE_NUM 1
#define DIRECTORIES 12
#define BLOCK_SIZE 512
#define DIR_NAME_LENGTH 14
#define MAX_ADDRESS 1023
#define MAX_DIR_COUNT 128
#define MIN_ARG_COUNT 2
#define MAX_ARG_COUNT 3
#define SUCCESS_CODE 0

// String constants
const char * SINGLE_DOT = ".";
const char * DOUBLE_DOT = "..";
const char * REPAIR_FLAG = "-r";

// Structure definitions
struct fsBlock
{
  uint total_size;
  uint block_count;
  uint inode_count;
};

struct inode_dir
{
  ushort inode_number;
  char node_name[DIR_NAME_LENGTH];
};

struct disk_inode
{
  short node_type;
  short minor_node;
  short major_node;
  short next_link;
  uint node_size;
  uint address_list[DIRECTORIES + 1];
};

// Global vars
bool *block_list;
unsigned int *dir_found;
unsigned int *inode_links;
bool *inode_ref;
bool *inode_used;
bool *used_blocks;
unsigned int inode_size;
unsigned int block_count;
struct fsBlock *block;
int *buffer;
void *xv6_image;

// Function prototypes
int calculatePwr(int base, int exp);

void printFatalError(const char *errorMsg);

void validateBitmap();

void validateInodes(struct disk_inode *node);

void validateDirectories(struct disk_inode *head_inode, struct disk_inode *dip);

void validateParentDirectories(struct disk_inode *head_inode, struct disk_inode *dip, int cur_node_number);

void validateRoot(struct disk_inode *head_inode, struct disk_inode *dip, int cur_node_number);

void validateIndirectAddresses(struct disk_inode *head_inode, struct disk_inode *dip, int cur_node_number);

void validateDirectAddresses(struct disk_inode *head_inode, struct disk_inode *dip, int cur_node_number);

void allocateBlocks();

int *generateBitmap();

// Error string constants
const char * INVALID_ARG_COUNT = "invalid argument count";
const char * IMAGE_NOT_FOUND = "image not found.";
const char * FSTAT_ERROR = "fstat failed.";
const char * MMAP_ERROR = "mmap failed.";
const char * BITMAP_MARK_NOT_IN_USE = "ERROR: bitmap marks block in use but it is not in use.";
const char * BAD_REF_COUNT = "ERROR: bad reference count for file.";
const char * INODE_MARKED_NOT_FOUND = "ERROR: inode marked use but not found in a directory.";
const char * INODE_REFERRED_MARKED_FREE = "ERROR: inode referred to in directory but marked free.";
const char * DUPLICATE_DIRECTORY = "ERROR: directory appears more than once in file system.";
const char * DIR_FORMAT_ERROR = "ERROR: directory not properly formatted.";
const char * PARENT_DIR_MISMATCH = "ERROR: parent directory mismatch.";
const char * INODE_BAD_INDIRECT_ADDR = "ERROR: bad indirect address in inode.";
const char * BITMAP_ADDRESS_MARKED_FREE = "ERROR: address used by inode but marked free in bitmap.";
const char * DUPLICATE_INDIRECT_ADDR = "ERROR: indirect address used more than once.";
const char * DUPLICATE_DIRECT_ADDR = "ERROR: direct address used more than once.";
const char * INODE_BAD_DIRECT_ADDR = "ERROR: bad direct address in inode.";
const char * ROOT_DIR_DNR = "ERROR: root directory does not exist.";
const char * BAD_INODE = "ERROR: bad inode.";