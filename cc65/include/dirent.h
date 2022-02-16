#ifndef __MEGA65LIB_DIRENT_H__
#define __MEGA65LIB_DIRENT_H__

extern unsigned char opendir(void);
extern struct m65_dirent *readdir(unsigned char);
extern void closedir(unsigned char);
extern void closeall(void);


struct m65_dirent {
  uint32_t d_ino;
  uint16_t d_off;
  uint32_t d_reclen;
  uint16_t d_type;
  char d_name[256];
};

#endif