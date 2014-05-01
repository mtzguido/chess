#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>

#define __maybe_unused  __attribute__((unused))

typedef	uint64_t	u64;
typedef	uint32_t	u32;
typedef	uint16_t	u16;
typedef	uint8_t		u8;
typedef	int64_t		i64;
typedef	int32_t		i32;
typedef	int16_t		i16;
typedef	int8_t		i8;

int isPrefix(char *a, char *b);
char pieceOf(char c);

#endif
