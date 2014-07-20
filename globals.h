#ifndef GLOBALS_H
#define GLOBALS_H
#include <iostream>

//comment the following to hide debugging output
// #define DEBUG

extern std::ostream cnull;

#ifdef DEBUG 
    #define dout (std::cerr)
#else
    #define dout (cnull)
#endif


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
#endif
