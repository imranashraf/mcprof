#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <cstdlib>

//Set the debugging level (0-3)
// 0   -> No Debug
// 1-3 -> Debug mode, while 3 is the most verbose
#define DEBUG 1

#if defined(_WIN64)
// 64-bit Windows uses LLP64 data model.
typedef unsigned long long  uptr;
typedef signed   long long  sptr;
#else
typedef unsigned long       uptr;
typedef signed   long       sptr;
#endif

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed   char       s8;
typedef signed   short      s16;
typedef signed   int        s32;
typedef signed   long long  s64;

typedef u8  FtnNo;

// Common defs.
#define INLINE inline
// Platform-specific defs.
#if defined(_MSC_VER)
# define ALWAYS_INLINE __forceinline
# define ALIAS(x)
# define ALIGNED(x) __declspec(align(x))
# define NOINLINE __declspec(noinline)
// # define NORETURN __declspec(noreturn)
# define LIKELY(x) (x)
# define UNLIKELY(x) (x)
# define PREFETCH(x) /* _mm_prefetch(x, _MM_HINT_NTA) */
#else
# define ALWAYS_INLINE inline __attribute__((always_inline))
# define ALIAS(x) __attribute__((alias(x)))
# define ALIGNED(x) __attribute__((aligned(x)))
# define NOINLINE __attribute__((noinline))
// # define NORETURN  __attribute__((noreturn))
# define LIKELY(x)     __builtin_expect(!!(x), 1)
# define UNLIKELY(x)   __builtin_expect(!!(x), 0)
# if defined(__i386__) || defined(__x86_64__)
// __builtin_prefetch(x) generates prefetchnt0 on x86
#  define PREFETCH(x) __asm__("prefetchnta (%0)" : : "r" (x))
# else
#  define PREFETCH(x) __builtin_prefetch(x)
# endif
#endif  // _MSC_VER

extern std::map <std::string,u16> Name2ID;
extern std::map <u16,std::string> ID2Name;
extern u16 GlobalID;
extern std::string UnknownFtn;
extern std::string UnknownObj;

#define ECHO(content) std::cerr << "[MCPROF] " << __FILE__ <<":"<< __LINE__ <<" "<< content << std::endl
#define VAR(v) " `" #v "': " << v << " "
#define VARS2(first, second) VAR(first) << " - " << VAR(second)
#define VARS3(first, second, third) VAR(first) << " - " << VARS2(second, third)
#define VARS4(first, second, third, fourth) VAR(first) << " - " << VARS3(second, third, fourth)

#define ADDR(v) " `" #v "': " << hex << "0x" << setw(12) << setfill ('0') << v << dec
#define FUNC(v)  ID2Name[(int)v] << "(" << (int)v << ")"

#if (DEBUG>0)
#define DECHO(content)                          ECHO(content)
#else
#define DECHO(content)
#endif

#if (DEBUG>=1)
#define D1ECHO(content)                          ECHO(content)
#else
#define D1ECHO(content)
#endif

#if (DEBUG>=2)
#define D2ECHO(content)                          ECHO(content)
#else
#define D2ECHO(content)
#endif

#if (DEBUG>=3)
#define D3ECHO(content)                          ECHO(content)
#else
#define D3ECHO(content)
#endif

// void Die();
#define Die(){\
    ECHO("Exiting, Good bye");\
    std::exit(EXIT_FAILURE);\
}

// Check macro
#define CHECK_IMPL(c1, op, c2) \
do { \
    u64 v1 = (u64)(c1); \
    u64 v2 = (u64)(c2); \
    if (UNLIKELY(!(v1 op v2))) {\
        ECHO("Check Failed" << VAR(c1) << #op << VAR(c2) );\
        Die(); \
    }\
}   while (false) \
/**/

#define CHECK(a)       CHECK_IMPL((a), !=, 0)
#define CHECK_EQ(a, b) CHECK_IMPL((a), ==, (b))
#define CHECK_NE(a, b) CHECK_IMPL((a), !=, (b))
#define CHECK_LT(a, b) CHECK_IMPL((a), <,  (b))
#define CHECK_LE(a, b) CHECK_IMPL((a), <=, (b))
#define CHECK_GT(a, b) CHECK_IMPL((a), >,  (b))
#define CHECK_GE(a, b) CHECK_IMPL((a), >=, (b))

#if (DEBUG>0)
#define DCHECK(a)       CHECK(a)
#define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#define DCHECK_NE(a, b) CHECK_NE(a, b)
#define DCHECK_LT(a, b) CHECK_LT(a, b)
#define DCHECK_LE(a, b) CHECK_LE(a, b)
#define DCHECK_GT(a, b) CHECK_GT(a, b)
#define DCHECK_GE(a, b) CHECK_GE(a, b)
#else
#define DCHECK(a)
#define DCHECK_EQ(a, b)
#define DCHECK_NE(a, b)
#define DCHECK_LT(a, b)
#define DCHECK_LE(a, b)
#define DCHECK_GT(a, b)
#define DCHECK_GE(a, b)
#endif

inline bool IsPowerOfTwo(uptr x)
{
    return (x & (x - 1)) == 0;
}

inline uptr RoundUpTo(uptr size, uptr boundary)
{
    CHECK(IsPowerOfTwo(boundary));
    return (size + boundary - 1) & ~(boundary - 1);
}

inline uptr RoundDownTo(uptr x, uptr boundary)
{
    return x & ~(boundary - 1);
}

inline bool IsAligned(uptr a, uptr alignment)
{
    return (a & (alignment - 1)) == 0;
}

#endif
