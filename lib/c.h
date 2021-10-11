#ifndef __robin_c
#define __robin_c


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#define local_persist static
#define internal      static
#define external      extern

#define false 0
#define true  1

#define assert(expression) \
	if (!(expression)) { \
		printf("bad assert at %s:%d\n", __FILE__, __LINE__); \
		printf(" -> assert(%s)\n", #expression); \
		__builtin_trap(); \
	}


#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE
#define GIGABYTE 1024*MEGABYTE
#define TERABYTE 1024*GIGABYTE

#define memory_alloc      malloc
#define struct_alloc(T)   memory_alloc(sizeof(T))
#define array_alloc(n, T) memory_alloc(n * sizeof(T))

#define memory_init(n)   memory_set(memory_alloc(n), n, 0);
#define struct_init(T)   memory_set(struct_alloc(T), sizeof(T), 0);
#define array_init(n, T) memory_set(array_alloc(n, T), n * sizeof(T), 0);


typedef int8_t	 s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t	 b8;
typedef int16_t  b16;
typedef int32_t  b32;
typedef int64_t  b64;
typedef float	 f32;
typedef double	 f64;
typedef char	 c8;
typedef uint16_t c16;
typedef uint32_t c32;


external void* memory_copy(void *dst, void *src, s32 n);
external void* memory_set(void *dst, s32 n, c8 value);


void* memory_copy(void *dst, void *src, s32 n) {
	c8 *cdst = (c8*) dst;
	c8 *csrc = (c8*) src;
	for (s32 i = 0; i < n; i++) cdst[i] = csrc[i];
	return dst;
}

void* memory_set(void *dst, s32 n, c8 value) {
	c8 *cdst = (c8*) dst;
	for (s32 i = 0; i < n; i++) cdst[i] = value;
	return dst;
}


#endif // __robin_c
