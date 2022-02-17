#ifndef __MEGA65LIB_SDCARD_H__
#define __MEGA65LIB_SDCARD_H__

#include <hal.h>


extern uint8_t sector_buffer[512];
extern unsigned char sdhc_card;


extern void mega65_clear_sector_buffer(void);
extern void mega65_sdcard_reset(void);
extern void mega65_fast(void);
extern void mega65_sdcard_open(void);
extern void mega65_sdcard_map_sector_buffer(void);
extern void mega65_sdcard_unmap_sector_buffer(void);
extern uint8_t mega65_sdcard_readsector(const uint32_t sector_number);
extern uint8_t mega65_sdcard_writesector(const uint32_t sector_number, uint8_t is_multi);
extern void mega65_sdcard_erase(const uint32_t first_sector,const uint32_t last_sector);

extern void	mega65_sdcard_read_file(char *filename,uint32_t load_address);
extern long mega65_sdcard_create_contiguous_file(char* name, long size, long root_dir_sector, long fat1_sector, long fat2_sector);

//extern uint32_t mega65_sdcard_getsize(void);
extern void mega65_sdcard_writenextsector(void);
extern void mega65_sdcard_writemultidone(void);

#endif

