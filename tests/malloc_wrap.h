#ifndef MALLOC_WRAP_H
#define MALLOC_WRAP_H

#include <stdlib.h>  /* Avoid breaking the usual definitions */
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	void *wrap_calloc(size_t nmemb, size_t size,
					  const char *file, unsigned int line, const char *func);
	void *wrap_malloc(size_t size,
					  const char *file, unsigned int line, const char *func);
	void *wrap_realloc(void *ptr, size_t size,
					   const char *file, unsigned int line, const char *func);
    void wrap_free(void *ptr,
                      const char *file, unsigned int line, const char *func);
    void* wrap_memcpy(void* d, const void* s, size_t n,
                      const char *file, unsigned int line, const char *func);
    void* wrap_memmove(void* d, const void* s, size_t n,
                      const char *file, unsigned int line, const char *func);
    void* wrap_set(void* d, const char v, size_t n,
                      const char *file, unsigned int line, const char *func);
	char *wrap_strdup(const char *s,
					  const char *file, unsigned int line, const char *func);

#ifdef __cplusplus
}
#endif

#ifdef USE_MALLOC_WRAPPERS

#  ifdef calloc
#    undef calloc
#  endif
#  define calloc(n, s)  wrap_calloc( (n), (s), __FILE__, __LINE__, __func__)

#  ifdef malloc
#    undef malloc
#  endif
#  define malloc(s)     wrap_malloc( (s),      __FILE__, __LINE__, __func__)

#  ifdef realloc
#    undef realloc
#  endif
#  define realloc(p, s) wrap_realloc((p), (s), __FILE__, __LINE__, __func__)

#  ifdef free
#    undef free
#  endif
#  define free(p)  wrap_free( (p), __FILE__, __LINE__, __func__)

#  ifdef memcpy
#    undef memcpy
#  endif
#  define memcpy(d,s,n)     wrap_memcpy( (d), (s), (n),  __FILE__, __LINE__, __func__)

#  ifdef memmove
#    undef memmove
#  endif
#  define memmove(d,s,n)     wrap_memmove( (d), (s), (n),  __FILE__, __LINE__, __func__)

#  ifdef memset
#    undef memset
#  endif
#  define memset(d,s,n)     wrap_memset( (d), (s), (n),  __FILE__, __LINE__, __func__)

#  ifdef strdup
#    undef strdup
#  endif
#  define strdup(s)     wrap_strdup( (s),      __FILE__, __LINE__, __func__)

#endif /* USE_MALLOC_WRAPPERS */

#endif /* MALLOC_WRAP_H */
