#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef USE_MALLOC_WRAPPERS
/* Don't wrap ourselves */
#  undef USE_MALLOC_WRAPPERS
#endif
#include "malloc_wrap.h"

void *wrap_calloc(size_t nmemb, size_t size,
				  const char *file, unsigned int line, const char *func)
{
	void *p = calloc(nmemb, size);
	if (NULL == p)
    {
		fprintf(stderr,
				"[%s] Failed to allocate %zd bytes at %s line %u: %s\n",
				func, nmemb * size, file, line, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void *wrap_malloc(size_t size,
				  const char *file, unsigned int line, const char *func)
{
	void *p = malloc(size);
	if (NULL == p)
    {
		fprintf(stderr,
				"[%s] Failed to allocate %zd bytes at %s line %u: %s\n",
				func, size, file, line, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void *wrap_realloc(void *ptr, size_t size,
				   const char *file, unsigned int line, const char *func)
{
	void *p = realloc(ptr, size);
	if (NULL == p)
    {
		fprintf(stderr,
				"[%s] Failed to allocate %zd bytes at %s line %u: %s\n",
				func, size, file, line, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void wrap_free(void *ptr,
                   const char *file, unsigned int line, const char *func)
{
    free(ptr);
}

void* wrap_memcpy(void* dst, const void* src, size_t size,
                    const char *file, unsigned int line, const char *func)
{
    void* dst2 = memcpy(dst,src,size);
    if(dst != dst2)
    {
        fprintf(stderr,
                "[%s] returned destination address is different, when copying %zd bytes at %s line %u: %s\n",
                func, size, file, line, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return dst2;
}

void* wrap_memmove(void* dst, const void* src, size_t size,
                    const char *file, unsigned int line, const char *func)
{
    void* dst2 = memmove(dst,src,size);
    if(dst != dst2)
    {
        fprintf(stderr,
                "[%s] returned destination address is different, when moving %zd bytes at %s line %u: %s\n",
                func, size, file, line, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return dst2;
}

void* wrap_memset(void* dst, const char val, size_t size,
                    const char *file, unsigned int line, const char *func)
{
    void* dst2 = memset(dst, val, size);
    if(dst != dst2)
    {
        fprintf(stderr,
                "[%s] returned destination address is different, when setting %zd bytes at %s line %u: %s\n",
                func, size, file, line, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return dst2;
}

char *wrap_strdup(const char *s,
				  const char *file, unsigned int line, const char *func)
{
	char *p = strdup(s);
	if (NULL == p)
    {
		fprintf(stderr,
				"[%s] Failed to allocate %zd bytes at %s line %u: %s\n",
				func, strlen(s), file, line, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

char *wrap_strcpy(char *d, const char *s,
                  const char *file, unsigned int line, const char *func)
{
    char *p = strcpy(d, s);
    if (d != p)
    {
        fprintf(stderr,
                "[%s] returned destination address is different, when copying %zd bytes at %s line %u: %s\n",
                func, strlen(s), file, line, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return p;
}
