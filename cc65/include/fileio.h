#ifndef __MEGA65LIB_FILEIO_H__
#define __MEGA65LIB_FILEIO_H__

struct file_descriptor_t {
#define FD_DISK_ID_FILE_CLOSED 0xFF
  unsigned char disk_id;
  unsigned long start_cluster;
  unsigned long current_cluster;
  unsigned char sector_in_cluster;
  unsigned long file_length;
  unsigned long buffer_position;
  unsigned long directory_cluster;
  unsigned short entry_in_directory;
  unsigned long buffer_address;
  unsigned short bytes_in_buffer;
  unsigned short offset_in_buffer;
};

struct m65_dirent {
  uint32_t d_ino;
  uint16_t d_off;
  uint32_t d_reclen;
  uint16_t d_type;
  char d_name[256];
};

extern void closedir(unsigned char);
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

extern unsigned char opendir(void);

extern struct m65_dirent* readdir(unsigned char);

extern char cdecl attachd81(char* image_name);

#endif