#ifndef MARKERS_H
#define MARKERS_H


#define __PIN_MAGIC(n) do {                                         \
        __asm__ __volatile__ ("movl %0, %%eax;                      \
                               xchg %%bx,%%bx"                      \
                               : /* no output registers */          \
                               : "r" (n) /* input register */       \
                               : "%eax"  /* clobbered register */   \
                              );                                    \
} while (0)

#define __PIN_MAGIC3(n, a1, a2) do {                                \
        __asm__ __volatile__ ("movl %0, %%eax;                      \
                               movl %1, %%ecx;                      \
                               movl %2, %%edx;                      \
                               xchg %%bx,%%bx"                      \
                               : /* no output registers */          \
                               : "r" (n), "r" (a1), "r" (a2)   /* input register */       \
                               : "%eax", "%ecx", "%edx"        /* clobbered register */   \
                              );                                    \
} while (0)

#define __PIN_CMD_MASK              0xff000000
#define __PIN_CMD_OFFSET            24
#define __PIN_ID_MASK               (~(__PIN_CMD_MASK))

#define __PIN_MAGIC_SIMICS          0
#define __PIN_MAGIC_START           1
#define __PIN_MAGIC_STOP            2
#define __PIN_MAGIC_END             3

#define __PIN_MAGIC_CMD_NOARG       0
#define __PIN_MAGIC_REGION          1
#define __PIN_MAGIC_MALLOC          2     /* track next malloc as object id <arg> */
#define __PIN_MAGIC_MALLOCM         3     /* track memory range <arg1> .. <arg1>+<arg2> as object id <arg> */
#define __PIN_MAGIC_ZONE_ENTER      0x04
#define __PIN_MAGIC_ZONE_EXIT       0x05

#define __PIN_MAKE_CMD_ARG(cmd, arg) ((cmd) << __PIN_CMD_OFFSET | ((arg) & __PIN_ID_MASK))
#define __PIN_CMD_ARG(cmd, arg)     __PIN_MAGIC(__PIN_MAKE_CMD_ARG(cmd, arg))


#define MCPROF_START()                 __PIN_MAGIC(__PIN_MAGIC_START)
#define MCPROF_STOP()                  __PIN_MAGIC(__PIN_MAGIC_STOP)

#define MCPROF_ZONE_ENTER(rid)         __PIN_CMD_ARG(__PIN_MAGIC_ZONE_ENTER, rid)
#define MCPROF_ZONE_EXIT(rid)          __PIN_CMD_ARG(__PIN_MAGIC_ZONE_EXIT, rid)

#endif // MARKERS_H
