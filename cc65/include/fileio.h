#ifndef __MEGA65LIB_FILEIO_H__
#define __MEGA65LIB_FILEIO_H__

extern void toggle_rom_write_protect();
extern void closeall(void);
extern void close(unsigned char fd);

// Returns file descriptor
extern unsigned char open(char *filename);

// Read upto one sector of data into the supplied buffer.
// Returns the number of bytes actually read.
extern unsigned short read512(unsigned char *buffer);

// Change working directory
// (only accepts one directory segment at a time
extern unsigned char chdir(char *filename);

// Change working directory to the root directory
extern unsigned char chdirroot(void);

#endif