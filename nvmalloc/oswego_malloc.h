#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void* nv_malloc(size_t);
void *nv_realloc(void *, size_t);
void *nv_calloc(size_t, size_t);

#ifdef __cplusplus
};
#endif



