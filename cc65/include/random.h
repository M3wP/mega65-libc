#ifndef __MEGA65LIB_RANDOM_H__
#define __MEGA65LIB_RANDOM_H__

#include <stdint.h>

extern uint32_t random32(uint32_t range);
extern uint16_t random16(uint16_t range);
extern uint8_t random8(uint8_t range);
extern void srand(uint32_t seed);
extern uint8_t rand8(uint8_t range);
extern uint16_t rand16(uint16_t range);
extern uint32_t rand32(uint32_t range);

#endif