#ifndef __MEGA65LIB_HAL_H__
#define __MEGA65LIB_HAL_H__

#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>

extern void mega65_fast(void);
extern void usleep(uint32_t micros);
extern char cdecl mega65_dos_exechelper(char* filename);


#endif
