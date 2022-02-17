#include <stdio.h>
#include <string.h>

#include "sdcard.h"
#include "hal.h"
#include "memory.h"
#include "time.h"
#include "targets.h"

/*
  Create a file in the root directory of the new FAT32 filesystem
  with the indicated name and size.

  The file will be created contiguous on disk, and the first
  sector of the created file returned.

  The root directory is the start of cluster 2, and clusters are
  assumed to be 4KB in size, to keep things simple.
  
  Returns first sector of file if successful, or -1 on failure.
*/
long mega65_sdcard_create_contiguous_file(char *name, long size,
					 long root_dir_sector,
					 long fat1_sector,
					 long fat2_sector)
{
  unsigned char i;
  unsigned short offset;
  unsigned short clusters;
  unsigned long start_cluster=0;
  unsigned long next_cluster;
  unsigned long contiguous_clusters=0;
  unsigned int fat_offset=0;
  int j;

  clusters=size/4096;
  if (size&4095) clusters++;

  for(fat_offset=0;fat_offset <= (fat2_sector-fat1_sector); fat_offset++) {
    mega65_sdcard_readsector(fat1_sector+fat_offset);
    contiguous_clusters=0;
    start_cluster=0;

    // Skip any FAT sectors with allocated clusters
    for(j=0;j<512;j++) if (sector_buffer[j]) break;
    if (j!=512) {
      continue;
    }
    
    for(offset=0;offset<512;offset+=4)
      {
	next_cluster=sector_buffer[offset];
	next_cluster|=((long)sector_buffer[offset+1]<<8L);
	next_cluster|=((long)sector_buffer[offset+2]<<16L);
	next_cluster|=((long)sector_buffer[offset+3]<<24L);
	if (!next_cluster) {
	  if (!start_cluster) {
	    start_cluster=(offset/4)+fat_offset*(512/4);
	  }
	  contiguous_clusters++;
	  if (contiguous_clusters==clusters) {
	    // End of chain marker
	    sector_buffer[offset+0]=0xff; sector_buffer[offset+1]=0xff;
	    sector_buffer[offset+2]=0xff; sector_buffer[offset+3]=0x0f;
	    break;
	  } else {
	    // Point to next cluster
	    uint32_t the_cluster=(fat_offset*(512/4)+(offset/4))+1;
	    sector_buffer[offset+0]=(the_cluster>>0)&0xff;
	    sector_buffer[offset+1]=(the_cluster>>8)&0xff;
	    sector_buffer[offset+2]=(the_cluster>>16)&0xff;
	    sector_buffer[offset+3]=(the_cluster>>24)&0xff;
	  }
	} else {
	  if (start_cluster) {
	    // write_line("ERROR: Disk space is fragmented. File not created.",0);
	    // 	    return 0;
	    // Not enough contiguous space in this FAT sector, so try the next
	    break;
	  }
	}
      }

    if (start_cluster&&(contiguous_clusters==clusters)) break;
    else {
    }
  }
  if ((!start_cluster)||(contiguous_clusters!=clusters)) {
    //    write_line("ERROR: Could not find enough free clusters in file system",0);
    return -1;
  }

  // Commit sector to disk (in both copies of FAT)
  mega65_sdcard_writesector(fat1_sector+fat_offset, 0);
  mega65_sdcard_writesector(fat2_sector+fat_offset, 0);
  
  mega65_sdcard_readsector(root_dir_sector);
  
  for(offset=0;offset<512;offset+=32)
    {
      if (sector_buffer[offset]>' ') continue;
      else break;
    }
  if (offset==512) {
    //    write_line("ERROR: First sector of root directory already full.",0);
    return -1;
  }

  // Build directory entry
  for(i=0;i<32;i++) sector_buffer[offset+i]=0x00;
  for(i=0;i<12;i++) sector_buffer[offset+i]=name[i];
  sector_buffer[offset+0x0b]=0x20; // Archive bit set
  sector_buffer[offset+0x1A]=start_cluster; 
  sector_buffer[offset+0x1B]=start_cluster>>8; 
  sector_buffer[offset+0x14]=start_cluster>>16; 
  sector_buffer[offset+0x15]=start_cluster>>24; 
  sector_buffer[offset+0x1C]=(size>>0)&0xff; 
  sector_buffer[offset+0x1D]=(size>>8L)&0xff; 
  sector_buffer[offset+0x1E]=(size>>16L)&0xff; 
  sector_buffer[offset+0x1F]=(size>>24l)&0xff;

  mega65_sdcard_writesector(root_dir_sector, 0);

  return root_dir_sector+(start_cluster-2)*8;
}

unsigned long root_dir_sector=0;
unsigned long fat1_sector=0;
unsigned long fat2_sector=0;
unsigned short reserved_sectors=0;
unsigned char sectors_per_cluster=0;
unsigned char fat_copies=0;
unsigned long sectors_per_fat=0;
unsigned long root_dir_cluster=0;

extern unsigned char sector_buffer[512];

//void sdcard_readsector(const uint32_t sector_number);

void mega65_serial_monitor_write(char *s)
{
  while(*s) {
    // There is almost certainly a better way to do this, but it works.
    POKE(0x380,*s);
    __asm__("lda $0380");

    // Use CLC in the spare instruction slot, in case assembler tries to
    // optimise a NOP away. 
    __asm__("sta $d643");
    __asm__("clc");
    s++;
  }
}

char hexchar2(unsigned char v)
{
  v=v&0xf;
  if (v<10) return '0'+v;
  return 0x41+v-10;
}

void hexout2(char *m,unsigned long v,int n)
{
  if (!n) return;
  do {
    m[n-1]=hexchar2(v);
    v=v>>4L;
    
  } while(--n);
}

char shbuf[11];
void serial_hex(unsigned long v)
{
  hexout2(shbuf,v,8);
  shbuf[8]=0x0d;
  shbuf[9]=0x0a;
  shbuf[10]=0;
  mega65_serial_monitor_write(shbuf);
}

void parse_partition_entry(const char i)
{
  char j;
  
  int offset=0x1be + (i<<4);

  char active=sector_buffer[offset+0];
  char shead=sector_buffer[offset+1];
  char ssector=sector_buffer[offset+2]&0x1f;
  int scylinder=((sector_buffer[offset+2]<<2)&0x300)+sector_buffer[offset+3];
  char id=sector_buffer[offset+4];
  char ehead=sector_buffer[offset+5];
  char esector=sector_buffer[offset+6]&0x1f;
  int ecylinder=((sector_buffer[offset+6]<<2)&0x300)+sector_buffer[offset+7];
  uint32_t lba_start,lba_size;

  for(j=0;j<4;j++) ((char *)&lba_start)[j]=sector_buffer[offset+8+j];
  for(j=0;j<4;j++) ((char *)&lba_size)[j]=sector_buffer[offset+12+j];

  switch (id) {
  case 0x0b: case 0x0c:
    // FAT32
    // lba_start has start of partition
    mega65_sdcard_readsector(lba_start);
    // reserved sectors @ $00e-$00f
    reserved_sectors=sector_buffer[0x0e]+(sector_buffer[0x0f]<<8L);
    // sectors per cluster @ $00d
    sectors_per_cluster=sector_buffer[0x0d];
    // number of FATs @ $010
    fat_copies=sector_buffer[0x10];
    // hidden sectors @ $01c-$01f
    // sectors per FAT @ $024-$027
    for(j=0;j<4;j++) ((char *)&sectors_per_fat)[j]=sector_buffer[0x24+j];
    // cluster of root directort @ $02c-$02f
    for(j=0;j<4;j++) ((char *)&root_dir_cluster)[j]=sector_buffer[0x2c+j];
    // $55 $AA signature @ $1fe-$1ff    
    
    // FATs begin at partition + reserved sectors
    // root dir = cluster 2 begins after 2nd FAT
    root_dir_sector=lba_start + reserved_sectors + sectors_per_fat * fat_copies;
    fat1_sector=lba_start + reserved_sectors;
    fat2_sector=lba_start + reserved_sectors + sectors_per_fat;
    
  }
  
#if 0
  printf("%02X%c : Start=%3d/%2d/%4d or %08X / End=%3d/%2d/%4d or %08X\n",
	 id,active&80?'*':' ',
	 shead,ssector,scylinder,lba_start,ehead,esector,ecylinder,lba_end);
#endif
  
}

// #define detect_target() (lpeek(0xffd3629))
// #define TARGET_UNKNOWN 0
// #define TARGET_MEGA65R1 1
// #define TARGET_MEGA65R2 2
// #define TARGET_MEGA65R3 3
// #define TARGET_MEGAPHONER1 0x21
// #define TARGET_NEXYS4 0x40
// #define TARGET_NEXYS4DDR 0x41
// #define TARGET_NEXYS4DDRWIDGET 0x42
// #define TARGET_WUKONG 0xFD
// #define TARGET_SIMULATION 0xFE

// struct m65_tm {
//   unsigned char tm_sec;    /* Seconds (0-60) */
//   unsigned char tm_min;    /* Minutes (0-59) */
//   unsigned char tm_hour;   /* Hours (0-23) */
//   unsigned char tm_mday;   /* Day of the month (1-31) */
//   unsigned char tm_mon;    /* Month (0-11) */
//   unsigned short tm_year;   /* Year - 1900 (in practice, never < 2000) */
//   unsigned char tm_wday;   /* Day of the week (0-6, Sunday = 0) */
//   int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
//   unsigned char tm_isdst;  /* Daylight saving time */
// };

//unsigned char db1, db2, db3;
//
//unsigned char lpeek_debounced(long address)
//{
// db1 = 0;
//  db2 = 1;
//  while (db1 != db2 || db1 != db3)
//  {
//    db1 = lpeek(address);
//    db2 = lpeek(address);
//    db3 = lpeek(address);
//  }
//  return db1;
//}

//unsigned char bcd_work;

// unsigned char unbcd(unsigned char in)
// {
//   bcd_work=0;
//   while(in&0xf0) {
//     bcd_work+=10;
//     in-=0x10;
//   }
//   bcd_work+=in;
//   return bcd_work;
// }

// void getrtc(struct m65_tm *tm)
// {
//   if (!tm) return;

//   tm->tm_sec=0;
//   tm->tm_min=0;
//   tm->tm_hour=0;
//   tm->tm_mday=0;
//   tm->tm_mon=0;
//   tm->tm_year=0;
//   tm->tm_wday=0;
//   tm->tm_isdst=0;
  
//   switch (detect_target()) {
//   case TARGET_MEGA65R2: case TARGET_MEGA65R3:
//     tm->tm_sec = unbcd(lpeek_debounced(0xffd7110));
//     tm->tm_min = unbcd(lpeek_debounced(0xffd7111));
//     tm->tm_hour = lpeek_debounced(0xffd7112);
//     if (tm->tm_hour&0x80) {
//       tm->tm_hour=unbcd(tm->tm_hour&0x3f);
//     } else {
//       if (tm->tm_hour&0x20) {
// 	tm->tm_hour=unbcd(tm->tm_hour&0x1f)+12;
//       } else {
// 	tm->tm_hour=unbcd(tm->tm_hour&0x1f);
//       }
//     }
//     tm->tm_mday = unbcd(lpeek_debounced(0xffd7113))-1;
//     tm->tm_mon = unbcd(lpeek_debounced(0xffd7114));
//     // RTC is based on 2000, not 1900
//     tm->tm_year = unbcd(lpeek_debounced(0xffd7115))+100;
//     tm->tm_wday = unbcd(lpeek_debounced(0xffd7116));
//     tm->tm_isdst= lpeek_debounced(0xffd7117)&0x20;
//     break;
//   case TARGET_MEGAPHONER1:
//     break;
//   default:
//     return;
//   }
// }



unsigned char fat32_open_file_system(void)
{
  unsigned char i;
  mega65_sdcard_readsector(0);
  if ((sector_buffer[0x1fe]!=0x55)||(sector_buffer[0x1ff]!=0xAA)) {
    return 255;
  } else {  
    for(i=0;i<4;i++) {
      parse_partition_entry(i);
    }
  }
  
}

unsigned long fat32_follow_cluster(unsigned long cluster)
{
  unsigned long r;
  // Read out the cluster number from the FAT
  mega65_sdcard_readsector(fat1_sector+(cluster/128));
  r=*(unsigned long *)sector_buffer[(cluster&127)<<2];
  return r;
}

unsigned long fat32_allocate_cluster(unsigned long cluster)
{
  unsigned long r;
  unsigned long fat_sector_num;
  unsigned short i;

  // Find free cluster
  for(fat_sector_num=0;fat_sector_num <= (fat2_sector-fat1_sector); fat_sector_num++) {
    mega65_sdcard_readsector(fat1_sector+fat_sector_num);
    for(i=0;i<512;i+=4) {
      if (sector_buffer[i]) continue;
      if (sector_buffer[i+3]) continue;
      if (sector_buffer[i+1]) continue;
      if (sector_buffer[i+2]) continue;
    }
    if (i<512) {
      // Found one
      r=fat_sector_num*128+(i>>2);
      *(unsigned long *)sector_buffer[i]=cluster;
      mega65_sdcard_writesector(fat1_sector+fat_sector_num,0);
      mega65_sdcard_writesector(fat2_sector+fat_sector_num,0);
      return r;
    }
  }
  return 0;
}


