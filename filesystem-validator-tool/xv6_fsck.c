// Author: Benjamin Lenington
// UW Email: lenington@wisc.edu
// Course: CS537 - Intro to Operating Systems
// Project: P5A - File System Checker

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "xv6_fsck.h"


int main(int argc, char *argv[])
{
  // Validate argument count
  if ((argc < MIN_ARG_COUNT ) || (argc > MAX_ARG_COUNT))
  {
    printFatalError(INVALID_ARG_COUNT);
  }

  // Check for optional repair flag
  if(strcmp(argv[1], REPAIR_FLAG) == 0){
    printf("Repair mode not yet supported\n");
  }

  // Attempt to open the image file in read only mode
  int fd = open(argv[1], O_RDONLY);
  if (fd < SUCCESS_CODE)
  {
    printFatalError(IMAGE_NOT_FOUND);
  }

  struct stat tmp_stat_buffer;
  if (fstat(fd, &tmp_stat_buffer) != SUCCESS_CODE)
  {
    printFatalError(FSTAT_ERROR);
  }

  xv6_image = mmap(NULL, tmp_stat_buffer.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (xv6_image == MAP_FAILED)
  {
    printFatalError(MMAP_ERROR);
  }

  // Pre allocate all required structures and generate bitmap
  allocateBlocks();
  buffer = generateBitmap(buffer);

  // Allocate inode structs and call helper functions
  struct disk_inode *cur_inode = (struct disk_inode *)(xv6_image + 2 * BLOCK_SIZE);
  struct disk_inode *current_inode = (struct disk_inode *)(xv6_image + 2 * BLOCK_SIZE);
  validateDirectories(cur_inode, current_inode);

  validateBitmap();

  // Allocate a new inode and call helper function
  struct disk_inode *node = cur_inode;
  validateInodes(node);

  // Return a success code once all validation functions have completed
  return 0;
}

void validateBitmap()
{
  for (int currentBlock = 0; currentBlock < block->total_size; currentBlock++)
  {
    if (currentBlock <= (block_count + 2))
    {
      continue;
    }
    if ((buffer[currentBlock] > 0) && !block_list[currentBlock])
    {
      printFatalError(BITMAP_MARK_NOT_IN_USE);
    }
  }
}

// @summary check for invalid or corrupt inode data
void validateInodes(struct disk_inode *node)
{
  int currentInode = 0;
  while (currentInode < block->inode_count)
  {
    if ((node->node_type == FILE_TYPE) && (inode_links[currentInode] != node->next_link))
    {
      printFatalError(BAD_REF_COUNT);
    }
    else if (!inode_ref[currentInode] && inode_used[currentInode])
    {
      printFatalError(INODE_MARKED_NOT_FOUND);
    }
    else if (!inode_used[currentInode] && inode_ref[currentInode])
    {
      printFatalError(INODE_REFERRED_MARKED_FREE);
    }
    else if ((node->node_type == DIR_TYPE) && (dir_found[currentInode] > 1))
    {
      printFatalError(DUPLICATE_DIRECTORY);
    }

    node++;
    currentInode++;
  }
}

//@summary validate all directories within the image and call helper functions
void validateDirectories(struct disk_inode *cur_inode, struct disk_inode *current_inode)
{
  int currentPos = 0;
  while (currentPos < block->inode_count)
  {
    validateRoot(cur_inode, current_inode, currentPos);

    if (current_inode->node_type != 0)
    {
      if (current_inode->node_type == DIR_TYPE)
      {
        validateParentDirectories(cur_inode, current_inode, currentPos);
      }
      validateIndirectAddresses(cur_inode, current_inode, currentPos);
      validateDirectAddresses(cur_inode, current_inode, currentPos);
    }
    current_inode++;
    currentPos++;
  }
}

//@summary verify that parent directories are correctly formatted
void validateParentDirectories(struct disk_inode *cur_inode, struct disk_inode *current_inode, int cur_node_number)
{
  struct inode_dir *parent_inode;
  bool parent_flag = false;
  struct inode_dir *current_dir = xv6_image + (BLOCK_SIZE * current_inode->address_list[0]);
  struct inode_dir *next_dir = current_dir + 1;

  if ((strcmp(next_dir->node_name, DOUBLE_DOT) + strcmp(current_dir->node_name, SINGLE_DOT)) != 0)
  {
    printFatalError(DIR_FORMAT_ERROR);
  }
  struct disk_inode *inode_parent = (cur_inode + next_dir->inode_number);

  unsigned int directory_count = (BLOCK_SIZE / sizeof(struct inode_dir));

  int currentDir = 0;
  while (currentDir < DIRECTORIES)
  {
    current_dir = xv6_image + (BLOCK_SIZE * current_inode->address_list[currentDir]);
    parent_inode = xv6_image + (BLOCK_SIZE * inode_parent->address_list[currentDir]);

    for (int counter = 0; counter < directory_count; counter++)
    {
      if (parent_inode->inode_number == cur_node_number)
      {
        parent_flag = true;
      }
      if (current_dir->inode_number != 0)
      {
        inode_ref[current_dir->inode_number] = true;
        struct disk_inode *node = (struct disk_inode *)(cur_inode + current_dir->inode_number);

        if (node->node_type == DIR_TYPE)
        {
          if ((strcmp(current_dir->node_name, DOUBLE_DOT) != SUCCESS_CODE) && (strcmp(current_dir->node_name, SINGLE_DOT) != SUCCESS_CODE))
          {
            dir_found[current_dir->inode_number]++;
          }
        }
        if (node->node_type == FILE_TYPE)
        {
          inode_links[current_dir->inode_number]++;
        }
      }
      current_dir++;
      parent_inode++;
    }
    currentDir++;
  }

  void *tmp_ptr = xv6_image + (BLOCK_SIZE * (current_inode->address_list[DIRECTORIES]));
  unsigned int *cur_block = (unsigned int *)tmp_ptr;

  tmp_ptr = xv6_image + (BLOCK_SIZE * (inode_parent->address_list[DIRECTORIES]));
  unsigned int *cur_parent_inode = (unsigned int *)tmp_ptr;
  currentDir = 0;

  while (currentDir < MAX_DIR_COUNT)
  {
    current_dir = xv6_image + (BLOCK_SIZE * (*cur_block));
    parent_inode = xv6_image + (BLOCK_SIZE * (*cur_parent_inode));
    for (int counter = 0; counter < directory_count; counter++)
    {
      if (current_dir->inode_number != 0)
      {
        inode_ref[current_dir->inode_number] = true;

        struct disk_inode *node = (struct disk_inode *)(cur_inode + current_dir->inode_number);
        if (node->node_type == FILE_TYPE)
        {
          inode_links[current_dir->inode_number]++;
        }
        if (node->node_type == DIR_TYPE)
        {
          if (strcmp(current_dir->node_name, DOUBLE_DOT) + strcmp(current_dir->node_name, SINGLE_DOT) != 0)
          {
            dir_found[current_dir->inode_number]++;
          }
        }
      }
      if (parent_inode->inode_number == cur_node_number)
      {
        parent_flag = true;
      }
      current_dir++;
      parent_inode++;
      currentDir++;
    }
    cur_block++;
    cur_parent_inode++;
  }

  // Check parent flag for directory mismatch
  if (parent_flag == false)
  {
    printFatalError(PARENT_DIR_MISMATCH);
  }
}

//@summary verify that the indirect addresses are valid
void validateIndirectAddresses(struct disk_inode *cur_inode, struct disk_inode *current_inode, int cur_node_number)
{
  void *tmp = xv6_image + (BLOCK_SIZE * (current_inode->address_list[DIRECTORIES]));
  unsigned int *current_block = (unsigned int *)tmp;
  for (int counter = 0; counter < MAX_DIR_COUNT; counter++)
  {
    if ((*(current_block) < 0) || (*(current_block) > MAX_ADDRESS))
    {
      printFatalError(INODE_BAD_INDIRECT_ADDR);
    }
    else if (buffer[*current_block] <= 0)
    {
      printFatalError(BITMAP_ADDRESS_MARKED_FREE);
    }

    block_list[*current_block] = true;
    if (used_blocks[*current_block] && *current_block != 0)
    {
      printFatalError(DUPLICATE_INDIRECT_ADDR);
    }
    else
    {
      used_blocks[*current_block] = true;
    }
    current_block++;
  }
}

//@summary verify that the direct addresses are valid
void validateDirectAddresses(struct disk_inode *cur_inode, struct disk_inode *current_inode, int cur_node_number)
{
  inode_used[cur_node_number] = 1;
  int current_index = 0;
  while (current_index < (DIRECTORIES + 1))
  {
    uint cur_address = current_inode->address_list[current_index];
    if ((cur_address < 0) || (cur_address > MAX_ADDRESS))
    {
      printFatalError(INODE_BAD_DIRECT_ADDR);
    }
    else if (buffer[cur_address] <= 0)
    {
      printFatalError(BITMAP_ADDRESS_MARKED_FREE);
    }
    else if ((cur_address != 0) && used_blocks[cur_address])
    {
      printFatalError(DUPLICATE_DIRECT_ADDR);
    }
    else
    {
      used_blocks[cur_address] = true;
    }

    block_list[cur_address] = true;
    current_index++;
  }
}

//@summary validate the root inode
void validateRoot(struct disk_inode *cur_inode, struct disk_inode *current_inode, int cur_node_number)
{
  if ((current_inode->node_type != DIR_TYPE) && (cur_node_number == ROOT_NODE_NUM))
  {
    printFatalError(ROOT_DIR_DNR);
  }
  else if ((current_inode->node_type != FILE_TYPE) && (current_inode->node_type != DIR_TYPE))
  {
    if ((current_inode->node_type != 0) && (current_inode->node_type != DEV_TYPE))
    {
      printFatalError(BAD_INODE);
    }
  }

  if (cur_node_number == ROOT_NODE_NUM && current_inode->node_type == DIR_TYPE)
  {
    struct inode_dir *current_dir = xv6_image + (BLOCK_SIZE * current_inode->address_list[0]);
    struct inode_dir *dir_next = current_dir + 1;

    if ((current_dir->inode_number == ROOT_NODE_NUM || dir_next->inode_number == ROOT_NODE_NUM))
    {
      if (current_dir->inode_number != dir_next->inode_number)
      {
        printFatalError(ROOT_DIR_DNR);
      }
    }
  }
}

// Helper functions


// @summary calculate and return the power of two numbers
int calculatePwr(int base, int exponent)
{
  int product = 1;
  for (int i = 0; i < exponent; ++i)
  {
    product = (product * base);
  }
  return product;
}


// @summary print a message to stderr before exiting with an error code
// @param error_message the message to print to stderr before exiting
void printFatalError(const char *error_message)
{
  // Print the error to stderr and exit with an error code
  fprintf(stderr, "%s\n", error_message);
  exit(1);
}


// @summary allocate a buffer, generate bitmap and return the buffer
int *generateBitmap(int *tmp_buf)
{
  tmp_buf = malloc(sizeof(int) * block->total_size);
  char *xv6_bm = xv6_image + (2 * BLOCK_SIZE) + (block_count * BLOCK_SIZE);
  int current_offset = 0;
  int current_block = 0;
  while (current_block < block->total_size)
  {
    tmp_buf[current_block] = (calculatePwr(2, current_offset) & *xv6_bm);
    if (current_offset < 7)
    {
      current_offset++;
    }
    else
    {
      current_offset = 0;
      xv6_bm++;
    }
    current_block++;
  }
  return tmp_buf;
}


// @summary calculate inode_size, block_count and allocate all arrays
void allocateBlocks()
{
  block = (struct fsBlock *)(xv6_image + BLOCK_SIZE);
  inode_size = sizeof(struct disk_inode) * block->inode_count;
  block_count = (inode_size / BLOCK_SIZE) + 1;

  block_list = malloc(sizeof(bool) * block->total_size);
  dir_found = malloc(sizeof(unsigned int) * block->inode_count);
  inode_links = malloc(sizeof(unsigned int) * block->inode_count);
  inode_ref = malloc(sizeof(bool) * block->inode_count);
  inode_used = malloc(sizeof(bool) * block->inode_count);
  used_blocks = malloc(sizeof(bool) * block->total_size);

  memset(block_list, false, block->total_size * sizeof(bool));
  memset(dir_found, 0, block->inode_count * sizeof(unsigned int));
  memset(inode_links, 0, block->inode_count * sizeof(unsigned int));
  memset(inode_ref, false, block->inode_count * sizeof(bool));
  memset(inode_used, false, block->inode_count * sizeof(bool));
  memset(used_blocks, false, block->total_size * sizeof(bool));
}
