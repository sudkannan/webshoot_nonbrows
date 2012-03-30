

#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

#define PROC_ID 994
#define CHUNK_ID 1
#define MAXSIZE 10 * 1024 * 1024

#define FASTA_DELETE_ME

struct rqst_struct {
    size_t bytes;
    const char* var;
    //unique id on how application wants to identify 
    //this chunk;
    int id;
    int pid;

    int ops;
    void *src;
    unsigned long mem;
    unsigned int order_id;

    //volatile flag
    int isVolatile;
    unsigned int mmap_id;
    unsigned long mmap_straddr;
};


void *allocate_mem(size_t size);
void nv_free(void *ptr);
size_t print_total_stats();
void *nvrealloc(void *, size_t);
void *pnvmalloc(size_t size, struct rqst_struct *rqst);
void *pnvread(size_t bytes, struct rqst_struct *rqst);
#ifdef __cplusplus
};
#endif


