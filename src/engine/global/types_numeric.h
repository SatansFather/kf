#pragma once

typedef signed char									i8;
typedef unsigned char								u8;
typedef short										i16;
typedef unsigned short								u16;
typedef unsigned int								u32;
typedef signed int									i32;
typedef long long									i64;
typedef unsigned long long							u64;
typedef float										f32;
typedef double										f64;
#if _COMPILER
typedef f64											GFlt;
#else
typedef f32											GFlt;
#endif


#define MAX_U8		255
#define MAX_I8		127
#define MIN_I8	   -128

#define MAX_U16		65535
#define MAX_I16		32767
#define MIN_I16	   -32768

#define MAX_U32		4294967295
#define MAX_I32		2147483647
#define MIN_I32    -2147483648

#define MAX_U64		18446744073709551615
#define MAX_I64		9223372036854775807
#define MIN_I64	   -9223372036854775808
