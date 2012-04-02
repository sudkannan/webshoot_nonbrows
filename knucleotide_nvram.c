/* The Computer Language Benchmarks Game

   http://shootout.alioth.debian.org/

 

   Based on bit encoding idea of C++ contribution of Andrew Moon

   Copy task division idea from Java entry, contributed by James McIlree

   Contributed by Petr Prokhorenkov

   Modified by Stefano Guidoni

*/

#define _GNU_SOURCE
#include "simple_hash3.h"
#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "IOtimer.h"
//#include "bench-framework.h"
//#include "benchstringio.h"
//#include "nvmalloc_wrap.h"

#define HT_SIZE 2000000

#ifdef ARRAYFILE
#define fgets_unlocked stringfile_fgets
#undef stdin
#define stdin stringfile_stdin
#else
#ifndef __GLIBC__
#define fgets_unlocked fgets
#endif
#endif


FILE *input_file;

typedef unsigned char uint8_t;
const uint8_t selector[] = { -1, 0,-1, 1, 3,-1,-1, 2 };
const char table[] = {'A', 'C', 'G', 'T'};


/* Thread pool implementation */

struct tp_entry {
    void *job;
    void *param;
};

struct tp {
    struct tp_entry *jobs;
    int capacity;
    int size;
    pthread_mutex_t mutex;
};

struct tp *
tp_create(int max_jobs) {
    struct tp *pool = malloc(sizeof(*pool));

    pool->jobs = malloc(sizeof(struct tp_entry)*max_jobs);
    pool->capacity = max_jobs;
    pool->size = 0;
    pthread_mutex_init(&pool->mutex, 0);

    return pool;
}

void
tp_destroy(struct tp *pool) {
    free(pool->jobs);
    pthread_mutex_destroy(&pool->mutex);
    free(pool);
}

void
tp_add_job(struct tp *pool, void *job, void *param) {
    if (pool->size < pool->capacity) {
        pool->jobs[pool->size].job = job;
        pool->jobs[pool->size].param = param;
        ++pool->size;
    }
}

void *
_tp_run(void *param) {
    struct tp *pool = param;

    for (;;) {
        void (*job)(void *) = 0;
        void *param = 0;

        pthread_mutex_lock(&pool->mutex);
        if (pool->size > 0) {
            job = pool->jobs[pool->size - 1].job;
            param = pool->jobs[pool->size - 1].param;
            --pool->size;
        }
        pthread_mutex_unlock(&pool->mutex);
        if (job == 0) {
            return 0;
        } else {
            job(param);
        }
    }
}

void
tp_run(struct tp *pool, int max_threads) {
    pthread_t threads[max_threads];
    for (int i = 0; i < max_threads; i++) {
        pthread_create(&threads[i], 0, &_tp_run, pool);
    }

    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], 0);
    }
}


char* readline(char *current, int *length, char* buffer){

        int idx =0;
        char* temp = strchr(buffer, '\n');
        if(!temp) return NULL;

        int len = (unsigned long)temp - (unsigned long)buffer;

        if(len < 1) return NULL;

        memcpy(current, buffer, len);
        current[len] =  '\n';
		*length = len;

        //fprintf(stdout," %s \n", current);

        buffer = buffer + len + 1;

        return buffer;
}


char *
read_stdin(int *stdin_size) {

    int input_size;
	char *temp = NULL; 
	struct rqst_struct rqst;
    struct stat stat;
    char *result; 
	int len = 0;

	//read everything into a buffer
	 char *buffer = NULL;

	//fstat(fileno(input_file), &stat);
    //input_size = stat.st_size;
	input_size = MAXSIZE;
	result = malloc(input_size);

    //fread (buffer,1,input_size,input_file);

     rqst.bytes = input_size;
     rqst.pid = PROC_ID;
     rqst.id = CHUNK_ID;

	// copy the file into the buffer:
	buffer =  (char *)pnvread(rqst.bytes,&rqst);
    assert(buffer);

    do {
	 buffer =  readline(result, &len, buffer);	
    } while (strncmp(result, ">THREE", 6));

    int read = 0;

	//buffer = readline(result + read, input_size - read, buffer);
	buffer = readline(result + read, &len , buffer);

	//len = strlen(result + read);
	while ( len ) {

        read += len;
        if (result[read - 1] == '\n') {
            read--;
        }
		//buffer = readline(result + read, input_size - read, buffer);
		len = 0;
		buffer = readline(result + read, &len, buffer);
	    //len = strlen(result + read);
        if (len == 0 || result[read] == '>' || buffer == NULL) {
            break;
        }
		//fprintf(stdout, "%s \n", result + read);
    }

    result[read++] = '>';
    result = realloc(result, read);
    *stdin_size = read;

    return result;
}

char *
read_stdin1(int *stdin_size) {
    int input_size;
	char *temp = NULL; 

    struct stat stat;
    fstat(fileno(input_file), &stat);
    input_size = stat.st_size;

    char *result = _malloc(input_size);

    do {
      assert(_fgets(result, input_size, input_file));
    } while (strncmp(result, ">THREE", 6));

    int read = 0;


    while (_fgets(result + read, input_size - read, input_file)) {

		
        int len = strlen(result + read);
        if (len == 0 || result[read] == '>') {
            break;
        }

		fprintf(stdout, "%s \n",result + read);

        read += len;
        if (result[read - 1] == '\n') {
            read--;
        }
    }



    result[read++] = '>';
    result = realloc(result, read);
    *stdin_size = read;

    return result;
}

static
inline char *
next_char(char *p) {
    do {
        ++p;
    } while (isspace(*p));

    return p;
}

static
inline uint64_t
push_char(uint64_t cur, uint8_t c) {
    return (cur << 2) + selector[(c & 7)];
}

uint64_t
pack_key(char *key, int len) {
    uint64_t code = 0;
    for (int i = 0; i < len; i++) {
        code = push_char(code, *key);
        key = next_char(key);
    }

    return code;
}

void
unpack_key(uint64_t key, int length, char *buffer) {
    int i;

    for (i = length - 1; i > -1; i--) {
        buffer[i] = table[key & 3];
        key >>= 2;
    }
    buffer[length] = 0;
}

void
generate_seqences(char *start, int length, int frame, struct ht_ht *ht) {
    uint64_t code = 0;
    uint64_t mask = (1ull << 2*frame) - 1;
    char *p = start;
    char *end = start + length;

    // Pull first frame.

    for (int i = 0; i < frame; i++) {
        code = push_char(code, *p);
        ++p;
    }
    ht_find_new(ht, code)->val++;

    while (p < end) {
        code = push_char(code, *p) & mask;
        ht_find_new(ht, code)->val++;
        ++p;
        if (*p & 8) {
            if (*p & 1) {
                ++p;
            } else
                break;
        }
    }
}

int
key_count_cmp(const void *l, const void *r) {
    const struct ht_node *lhs = l, *rhs = r;

    if (lhs->val != rhs->val) {
        return rhs->val - lhs->val;
    } else {
        // Overflow is possible here,

        // so use comparisons instead of subtraction.

        if (lhs->key < rhs->key) {
            return -1;
        } else if (lhs->key > rhs->key) {
            return 1;
        } else {
            return 0;
        }
    }
}

struct print_freqs_param {
    char *start;
    int length;
    int frame;
    char *output;
    int output_size;
};

struct ht_node *
ht_values_as_vector(struct ht_ht *ht) {
    struct ht_node *v = malloc(ht->items*sizeof(struct ht_node));
    struct ht_node *n = ht_first(ht);

    for (int i = 0; i < ht->items; i++) {
        v[i] = *n;
        n = ht_next(ht);
    }

    return v;
}

void
print_freqs(struct print_freqs_param *param) {
    char *start = param->start;
    int length = param->length;
    int frame = param->frame;
    char *output = param->output;
    int output_size = param->output_size;

    struct ht_ht *ht = ht_create(32);
    char buffer[frame + 1];
    int output_pos = 0;

    generate_seqences(start, length, frame, ht);
    
    struct ht_node *counts = ht_values_as_vector(ht);
    int size = ht->items;

    qsort(counts, size, sizeof(struct ht_node), &key_count_cmp);

    int total_count = 0;
    for (int i = 0; i < size; i++) {
        total_count += counts[i].val;
    }

    for (int i = 0; i < size; i++) {
        unpack_key(counts[i].key, frame, buffer);
        output_pos += snprintf(output + output_pos, output_size - output_pos,
                "%s %.3f\n", buffer, counts[i].val*100.0f/total_count);
    }

    free(counts);
    ht_destroy(ht);
}

struct print_occurences_param {
    char *start;
    int length;
    char *nuc_seq;
    char *output;
    int output_size;
};

void
print_occurences(struct print_occurences_param *param) {
    char *start = param->start;
    int length = param->length;
    char *nuc_seq = param->nuc_seq;
    char *output = param->output;
    int output_size = param->output_size;
    int nuc_seq_len = strlen(nuc_seq);
    struct ht_ht *ht = ht_create(HT_SIZE);

    generate_seqences(start, length, nuc_seq_len, ht);

    uint64_t key = pack_key(nuc_seq, nuc_seq_len);
    int count = ht_find_new(ht, key)->val;
    snprintf(output, output_size, "%d\t%s\n", count, nuc_seq);
    
    ht_destroy(ht);
}

int
get_cpu_count(void) {
#ifdef __GLIBC__
    cpu_set_t cpu_set;

    CPU_ZERO(&cpu_set);
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);

    return CPU_COUNT(&cpu_set);
#else
    return 1;
#endif
}

#define MAX_OUTPUT 1024


int
knucleotide_main(void) {
    int stdin_size;
    char *stdin_mem = read_stdin(&stdin_size);
    int cpu_count = get_cpu_count();

    char output_buffer[7][MAX_OUTPUT];

#   define DECLARE_PARAM(o, n) {\
    .start = stdin_mem, \
    .length = stdin_size, \
    .frame = n,\
    .output = output_buffer[o],\
    .output_size = MAX_OUTPUT }

    struct print_freqs_param freq_params[2] = {
        DECLARE_PARAM(0, 1),
        DECLARE_PARAM(1, 2)
    }; 

#   undef DECLARE_PARAM


#   define DECLARE_PARAM(o, s) {\
    .start = stdin_mem, \
    .length = stdin_size, \
    .nuc_seq = s,\
    .output = output_buffer[o],\
    .output_size = MAX_OUTPUT }

    struct print_occurences_param occurences_params[5] = {
        DECLARE_PARAM(2, "GGT"),
        DECLARE_PARAM(3, "GGTA"),
        DECLARE_PARAM(4, "GGTATT"),
        DECLARE_PARAM(5, "GGTATTTTAATT"),
        DECLARE_PARAM(6, "GGTATTTTAATTTATAGT")
    };

#   undef DECLARE_PARAM


    struct tp *tp = tp_create(7);

    for (int i = 0 ; i < 2; i++) {
        tp_add_job(tp, &print_freqs, &freq_params[i]);
    }
    for (int i = 0 ;i <  5; i++) {
        tp_add_job(tp, &print_occurences, &occurences_params[i]);
    }

    tp_run(tp, cpu_count + 1);

    tp_destroy(tp);

    volatile int len;//keep the compiler from optimizing away
    for (int i = 0; i < 2; i++) {
      //printf("%s\n", output_buffer[i]);
      len = strlen(output_buffer[i]);
    }
    for (int i = 2; i < 7; i++) {
      //printf("%s", output_buffer[i]);
      len = strlen(output_buffer[i]);
    }

    free(stdin_mem);

    return 0;
}

int run_knucleotide(int ignored) {

	fprintf(stderr,"%s \n", KNUCLEO_INFILE);

  input_file = fopen((char *)KNUCLEO_INFILE, "r");
  if (!input_file) return -1;


  knucleotide_main();

  fclose(input_file);

  return 0;
}
