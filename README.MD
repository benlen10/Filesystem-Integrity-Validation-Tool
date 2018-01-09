## Filesystem Integrity Validation Tool

### Brief Description: 
- Based on a project for CS537 - Operating Systems at The University of Wisconsin Madison

- An xv6 file system validator program for Linux environments, written in C. Scans an XV6 file system image and checks for file corruption or integrity violations.

#### Error Detection Summary: 
- This scanner will detect several filesystem integrity violations including improper directory format, invalid inodes, invalid direct or indirect addresses, bitmap/inodes inconsistencies, corrupted/missing root directories and duplicate addresses.

#### Program Structure: 
- The validation process is broken down into multiple helper functions which each check for different error types within the xv6 filesystem. 

#### Program Usage: 
- The program accepts a single argument which is the xv6 disk image name. Optionally, “-r” can be specified before the xv6 image name to enter repair mode (Not yet implemented – see description below)

#### Implementation: 
- I implemented an additional check for parent directory mismatches using a flag within the validateParentDirectories function. I began to implement the image repair functionality to repair “inode marked use but not found” error but encountered significant difficulties repairing these errors and did have not time to continue debugging. I removed the partial repair code and replaced it with a placeholder print statement when the program is run with the “-r” repair flag. 
