#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "oswego_malloc.h"

//#define USE_NVMALLOC
//#define USE_STATS


extern void* nv_malloc(size_t);
extern void *nv_realloc(void *, size_t);
extern void *nv_calloc(size_t, size_t);

extern void hash_insert( unsigned long key, size_t val);
extern size_t hash_find( unsigned long key);
extern void hash_delete( unsigned long key);
extern size_t find_hash_total();

size_t print_total_stats() {

	size_t total_alloc =0;

	total_alloc= find_hash_total();

	 return total_alloc;	

}


int input_id =0;
int tid = 1;
char *addr = NULL;
size_t region_size = 65536 * 3000;
int d = -1;
size_t val = 0;
size_t total_size = 0;

void *allocate_mem(size_t size) {

    int flgPersist = 0;

	total_size += size;

//#ifdef USE_NVMALLOC
//	addr = (char *)nv_malloc(size);
//#else
	addr = (char *)malloc(size);
//#endif

#ifdef USE_STATS
	//fprintf(stderr,"%d %zu\n", ++input_id, total_size );
    if(!addr){
        fprintf(stderr,"NVmalloc allocation failed \n");
        return NULL;
    }else{
		hash_insert((unsigned long)addr, size);
    }
#endif
    return addr;
}

void *pnvmalloc(size_t size, struct rqst_struct *rqst) {

    int flgPersist = 0;

	if(!rqst) {
		perror("failed pnvmalloc \n");
		return NULL;
	}

#ifdef USE_NVMALLOC
	addr = (char *)pnv_malloc(size, rqst);
#else
	addr = (char *)malloc(size);
#endif

#ifdef USE_STATS
    if(!addr){
        fprintf(stderr,"NVmalloc allocation failed \n");
        return NULL;
    }else{
		hash_insert((unsigned long)addr, size);
    }
#endif
    return addr;
}

void *pnvread(size_t size, struct rqst_struct *rqst) {

    int flgPersist = 0;

	if(!rqst) {
		perror("failed pnvmalloc \n");
		return NULL;
	}

#ifdef USE_NVMALLOC
	addr = (char *)pnv_read(size, rqst);
#else
	addr = (char *)malloc(size);
#endif

#ifdef USE_STATS
	//fprintf(stderr,"%d %zu\n", ++input_id, total_size );
    if(!addr){
        fprintf(stderr,"NVmalloc allocation failed \n");
        return NULL;
    }else{
		hash_insert((unsigned long)addr, size);
    }
#endif
    return addr;
}




void *nvcalloc(size_t nelemnts, size_t elemnt_sz) {

    int flgPersist = 0;

	total_size += nelemnts* elemnt_sz;
    ++input_id;

#ifdef USE_NVMALLOC    
	addr = (char *)nv_calloc(nelemnts, elemnt_sz);
#else
	addr = calloc(nelemnts, elemnt_sz);
#endif

    if(!addr){
        fprintf(stderr,"nvcalloc allocation failed \n");
        return NULL;
    }/*else{
		hash_insert((unsigned long)addr, nelemnts* elemnt_sz);
    }*/
	//fprintf(stderr,"returning address %lu \n",(unsigned long)addr);
    return (void *)addr;
}



void *nvrealloc(void *orig_ptr,  size_t size) {

    unsigned long addr;
	void *new_ptr = NULL;

#ifdef USE_NVMALLOC
     new_ptr = (void *)nv_realloc(orig_ptr, size);
#else
	new_ptr = realloc(orig_ptr, size);	
#endif

     if(new_ptr == NULL){
		fprintf(stderr,"nv_realloc failed \n");
	 }
#ifdef USE_STATS
	addr = (unsigned long)orig_ptr;
    if(!addr){
        fprintf(stderr,"NVmalloc reallocation failed \n");
        return NULL;
    }else{
		if(hash_find(addr)) {

			if( addr == (unsigned long)new_ptr) {

				hash_insert(addr, size);
			}else {
	
				hash_delete(addr);
				hash_insert((unsigned long)new_ptr, size);
			}
		}else{
			hash_insert((unsigned long)new_ptr, size);
		}
	fprintf(stderr,"nvrealloc return %zu \n",size);
	}
#endif

    return new_ptr;
}

void nv_free(void *addr) {

#ifdef USE_NVMALLOC
	//nvfree(addr);
#else
	free(addr);
#endif
}




