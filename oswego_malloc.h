#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void *nv_malloc(size_t);
void *nv_realloc(void *, size_t);
void *nv_calloc(size_t, size_t);
void *pnv_malloc(size_t , struct rqst_struct *);
void *pnv_read(size_t bytes, struct rqst_struct *rqst);
#ifdef __cplusplus
};
#endif



