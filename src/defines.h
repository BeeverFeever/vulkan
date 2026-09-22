#pragma once

#include <stddef.h>
#include <stdint.h>

typedef ptrdiff_t    ptrdiff;
typedef uintptr_t    uintptr;
typedef unsigned int Uint;
typedef char         byte;
typedef uint8_t      u8;
typedef uint16_t     u16;
typedef uint32_t     u32;
typedef uint64_t     u64;
typedef int8_t       i8;
typedef int16_t      i16;
typedef int32_t      i32;
typedef int64_t      i64;
typedef float        f32;
typedef double       f64;
typedef ptrdiff_t    Size;
typedef size_t       Usize;

// NOTE: this shadows the `sizeof` keyword everywhere this header is visible
// (including in headers included afterwards), so every `sizeof` in the
// translation unit becomes a Size (signed) instead of a size_t (unsigned).
// It only fires when written as `sizeof(x)`; `sizeof x` without parens is
// untouched and stays size_t, so avoid mixing the two styles.
#define sizeof(x) (Size)sizeof(x)
#define alignof(x) (Size)_Alignof(x)
#define lengthof(a) (sizeof(a) / sizeof(*(a)))

#define KB(s) (long long)((s) * 1024)
#define MB(s) (long long)(KB(s) * 1024)
#define GB(s) (long long)(MB(s) * 1024)

#define str(cstr) str_from_cstr(cstr)

static_assert(sizeof(Uint) == 4, "Expected Uint to be 4 bytes.");
static_assert(sizeof(byte) == 1, "Expected byte to be 1 byte.");
static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");
static_assert(sizeof(i8) == 1, "Expected i8 to be one byte.");
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");
static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");
static_assert(sizeof(Size) == 8, "Expected Size to be 8 bytes.");
static_assert(sizeof(Usize) == 8, "Expected Usize to be 8 bytes.");

