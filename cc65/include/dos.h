#ifndef __MEGA65LIB_DOS_H__
#define __MEGA65LIB_DOS_H__

extern char cdecl mega65_dos_chdirroot(void);
extern char cdecl mega65_dos_chdir(char* dirname);
extern char cdecl mega65_dos_attachd81(char* image_name);
extern char cdecl mega65_dos_exechelper(char* filename);

extern void fastcall mega65_save_zp(void);
extern void fastcall mega65_restore_zp(void);

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

#endif