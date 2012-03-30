/* The Computer Language Benchmarks Game
   http://shootout.alioth.debian.org/

   contributed by Petr Prokhorenkov
*/

#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "IOtimer.h"
#include "nvmalloc_wrap.h"
//#include "bench-framework.h"
//#include "benchstringio.h"

#define LINE_LENGTH 60 
#define STDOUT 1

#ifdef ARRAYFILE
#define fgets_unlocked stringfile_fgets
#undef stdin
#define stdin stringfile_stdin
#else
#ifndef __GLIBC__
#define fgets_unlocked fgets
#endif
#endif

#define INFILE "fasta_output"
#define OUTFILE "revcomp_output"


FILE *ouput_file, *input_file;

//We have nvram output file
char *nv_outfile = NULL;


typedef struct lookup {
    uint8_t bytes[256];
    uint16_t pairs[256*256];
} lookup_t;

void
init_lookup(const uint8_t *from, const uint8_t *to, lookup_t *lookup) {
    for (int i = 0; i < 256; ++i) {
        lookup->bytes[i] = i;
    }
    for (; *from && *to; ++from, ++to) {
        lookup->bytes[tolower(*from)] = *to;
        lookup->bytes[toupper(*from)] = *to;
    }

    for (int i = 0; i != 256; ++i) {
        for (int j = 0; j != 256; ++j) {
            lookup->pairs[(i << 8) + j] =
                ((uint16_t)lookup->bytes[j] << 8) + lookup->bytes[i];
        }
    }
}

typedef struct job {
    struct job *prev_job;

    pthread_mutex_t *io_mutex;
    pthread_cond_t condition;
    int finished;

    const uint8_t *header_begin;
    const uint8_t *header_end;
    const uint8_t *body_begin;
    const uint8_t *body_end;
    const lookup_t *lookup;
    pthread_t thread;
    uint8_t *buffer;
    int free_buffer;
} job_t;

off_t get_file_size(FILE *f) {
    long begin = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f) - begin;
    fseek(f, begin, SEEK_SET);

    return size;
}

void persistent_write(const void *ptr, size_t size) {
#ifdef ARRAYFILE
  assert(_fwrite(ptr, size, 1, ouput_file));
#else

    struct rqst_struct rqst;

    rqst.bytes = size;
    rqst.pid = PROC_ID;
    rqst.id = CHUNK_ID+2;

    nv_outfile = malloc(size); //pnvmalloc(size, &rqst);
	if(!nv_outfile) {
		fprintf(stderr,"persistent_write: outfile allocation failed \n");
		return;
	}

	memcpy(nv_outfile,ptr, size);

#endif
	
	//TODO Commit statement

}

void
persistent_write_orig(int fd, const void *ptr, size_t size) {
#ifdef ARRAYFILE
  assert(_fwrite(ptr, size, 1, ouput_file));
#else

   fd = fileno(ouput_file);

    while (size > 0) {
        ssize_t result = _write(fd, ptr, size);

        assert(result >= 0);

        size -= result;
    }
#endif
}





size_t
reverse_complement(
        const uint8_t *begin,
        const uint8_t *end,
        const lookup_t *lookup,
        uint8_t *buffer) {
    size_t size = end - begin;
    const uint8_t *read_p = (const uint8_t *)end;
    uint8_t *buffer_begin = buffer;

    while (size >= LINE_LENGTH) {
        uint16_t *pair_buffer = (uint16_t *)buffer;
        const uint16_t *read_pair = (const uint16_t *)read_p;

        for (size_t i = 0; i < LINE_LENGTH; i += 2) {
            *pair_buffer++ = lookup->pairs[*--read_pair];
        }

        read_p -= LINE_LENGTH&(~1);
        buffer += LINE_LENGTH&(~1);

        if (LINE_LENGTH % 2 != 0) {
            *buffer++ = *--read_p;
        }

        *buffer++ = '\n';
        size -= LINE_LENGTH;
    }

    if (size > 0) {
       while (read_p > begin) {
           *buffer++ = lookup->bytes[*--read_p];
       }
       *buffer++ = '\n';
    }

    return buffer - buffer_begin;
}

size_t round_by(size_t a, size_t b) {
    return a - a%b;
}

void
process_block(
        job_t *job) {
    const uint8_t *header_begin = job->header_begin;
    const uint8_t *header_end = job->header_end;
    const uint8_t *body_begin = job->body_begin;
    const uint8_t *body_end = job->body_end;
    const lookup_t *lookup = job->lookup;
    pthread_mutex_t *io_mutex = job->io_mutex;
    pthread_cond_t *condition = &job->condition;
    uint8_t *buffer = job->buffer;
	struct rqst_struct rqst;


    size_t size = reverse_complement(
            body_begin, body_end,
            lookup,
            buffer);

    pthread_mutex_lock(io_mutex);
    if (job->prev_job) {
        if (!job->prev_job->finished) {
            pthread_cond_wait(
                    &job->prev_job->condition, io_mutex);
        }
    }

    rqst.bytes = header_end - header_begin + size;
    rqst.pid = PROC_ID;
    rqst.id = CHUNK_ID+2;

    nv_outfile = pnvmalloc(rqst.bytes, &rqst);
    if(!nv_outfile) {
        fprintf(stderr,"process_block: outfile allocation failed \n");
        return;
    }

    if (header_begin && header_end) {

//        persistent_write(
//                STDOUT, header_begin, header_end - header_begin);

		 //NVRAM changes	
		 //persistent_write(header_begin, header_end - header_begin);
		 memcpy(nv_outfile, header_begin, header_end - header_begin);

    }

    //NVRAM changes
    //persistent_write(STDOUT, buffer, size);
	 int len = header_end - header_begin;
	 memcpy(nv_outfile + len,  buffer, size);


    job->finished = 1;
    pthread_cond_signal(condition);
    pthread_mutex_unlock(io_mutex);
}

job_t *detach_job(
        const uint8_t *header_begin, const uint8_t *header_end,
        const uint8_t *body_begin, const uint8_t *body_end,
        const lookup_t *lookup,
        pthread_mutex_t *mutex,
        job_t *prev_job,
        uint8_t *buffer,
        int free_buffer) {
    job_t *job = calloc(1, sizeof(*job));
    assert(job);

    job->io_mutex = mutex;
    pthread_cond_init(&job->condition, 0);
    job->header_begin = header_begin;
    job->header_end = header_end;
    job->body_begin = body_begin;
    job->body_end = body_end;
    job->lookup = lookup;
    job->buffer = buffer;
    job->prev_job = prev_job;
    job->free_buffer = free_buffer;

    pthread_create(&job->thread, 0, (void *(*)(void *))&process_block, job);

    return job;
}

job_t *
do_process_block(
        const uint8_t *header_begin, const uint8_t *header_end,
        const uint8_t *body_begin, const uint8_t *body_end,
        const lookup_t *lookup,
        pthread_mutex_t *mutex,
        job_t *prev_job) {
    uint64_t body_size = body_end - body_begin;
    uint64_t tail_len = round_by(body_size/2, LINE_LENGTH);
    const uint8_t *split_at = body_end - tail_len;

    uint8_t *buffer = malloc(
            body_size*(LINE_LENGTH + 1)/LINE_LENGTH + LINE_LENGTH);
    assert(buffer);

    prev_job = detach_job(
            header_begin, header_end,
            split_at, body_end,
            lookup,
            mutex,
            prev_job,
            buffer,
            1);
    prev_job = detach_job(
            0, 0,
            body_begin, split_at,
            lookup,
            mutex,
            prev_job,
            buffer + tail_len*(LINE_LENGTH + 1)/LINE_LENGTH,
            0);

    return prev_job;
}


int revcomp_main(void) {

    //setvbuf(input_file, 0, _IOFBF, 1024*1024);

    //long stdin_size = get_file_size(input_file);

	long lSize;
	size_t result;
	struct rqst_struct rqst;

    rqst.bytes = MAXSIZE;
    rqst.pid = PROC_ID;
    rqst.id = CHUNK_ID;

	lSize = rqst.bytes;

	fprintf(stdout, "entering revcomp_main \n");

	// copy the file into the buffer:
	uint8_t *buffer =  pnvread(lSize,&rqst);

    //uint8_t *buffer = calloc(stdin_size + LINE_LENGTH, 1);
    assert(buffer);
    lookup_t lookup;
    pthread_mutex_t mutex;
    job_t *job = 0;

    init_lookup(
            (const uint8_t *)"acbdghkmnsrutwvy",
            (const uint8_t *)"TGVHCDMKNSYAAWBR",
            &lookup);

    pthread_mutex_init(&mutex, 0);

    uint8_t *current = buffer;
    uint8_t *end = buffer + lSize; 
    uint8_t *header_begin = 0;
    uint8_t *header_end = 0;
    uint8_t *body_begin = 0;

	 
	fprintf(stderr,"Current %s \n",current);


        if (current[0] == '>') {
            if (body_begin != 0) {
                job = do_process_block(
                        header_begin, header_end,
                        body_begin, current,
                        &lookup,
                        &mutex,
                        job); 
            }
            size_t len = strlen((const char *)current);
            header_begin = current;
            header_end = current + len;
            current += len;
            body_begin = current;
        } else {
            if (current[LINE_LENGTH] == '\n') {
                current += LINE_LENGTH;
            } else {
                size_t len = strlen((const char *)current);
                if (current[len - 1] == '\n') {
                    --len;
                }
                current += len;
            }
        }

    if (body_begin != 0) {
        job = do_process_block(
                header_begin, header_end,
                body_begin, current,
                &lookup,
                &mutex,
                job); 
    }

    while (job) {
        pthread_join(job->thread, 0);

        if (job->free_buffer) {
            nv_free(job->buffer);
        }
        job = job->prev_job;
    }

	fprintf(stdout, "revcomp main returning \n");
    
    /*free(buffer);*/

    return 0;
}




int
revcomp_main1(void) {

    setvbuf(input_file, 0, _IOFBF, 1024*1024);

    long stdin_size = get_file_size(input_file);

    uint8_t *buffer = calloc(stdin_size + LINE_LENGTH, 1);
    assert(buffer);
    lookup_t lookup;
    pthread_mutex_t mutex;
    job_t *job = 0;

    init_lookup(
            (const uint8_t *)"acbdghkmnsrutwvy",
            (const uint8_t *)"TGVHCDMKNSYAAWBR",
            &lookup);

    pthread_mutex_init(&mutex, 0);

    uint8_t *current = buffer;
    uint8_t *end = buffer + stdin_size;
    uint8_t *header_begin = 0;
    uint8_t *header_end = 0;
    uint8_t *body_begin = 0;

    while (fgets_unlocked((char *)current, end - current, input_file)) {
        if (current[0] == '>') {
            if (body_begin != 0) {
                job = do_process_block(
                        header_begin, header_end,
                        body_begin, current,
                        &lookup,
                        &mutex,
                        job); 
            }
            size_t len = strlen((const char *)current);
            header_begin = current;
            header_end = current + len;
            current += len;
            body_begin = current;
        } else {
            if (current[LINE_LENGTH] == '\n') {
                current += LINE_LENGTH;
            } else {
                size_t len = strlen((const char *)current);
                if (current[len - 1] == '\n') {
                    --len;
                }
                current += len;
            }
        }
    }

    if (body_begin != 0) {
        job = do_process_block(
                header_begin, header_end,
                body_begin, current,
                &lookup,
                &mutex,
                job); 
    }

    while (job) {
        pthread_join(job->thread, 0);

        if (job->free_buffer) {
            free(job->buffer);
        }
        job = job->prev_job;
    }
    
    free(buffer);

    return 0;
}

char *arrayfile_join2_old (FILE *pFile)  {
  long lSize;
  char * buffer;
  size_t result;

  if(!pFile) fprintf(stderr,"invalid file \n");


  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = _fread (buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */

  // terminate
  fclose (pFile);

  return buffer;

  return 0;
}

char *arrayfile_join2 (FILE *pFile)  {

	long lSize;
	char * buffer;
	size_t result;
	struct rqst_struct rqst;

    lSize = 10125;
	rqst.bytes = lSize;
	rqst.pid = PROC_ID;
	rqst.id = CHUNK_ID+2;

    buffer =  pnvmalloc(rqst.bytes, &rqst);
    if(!buffer) {
        fprintf(stderr,"arrayfile_join2: outfile allocation failed \n");
        return;
    }

	return buffer;

  // allocate memory to contain the whole file:
  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  buffer =  (char *)allocate_mem(lSize);
  if (buffer == NULL) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */
  return buffer;

  return 0;
}





int run_revcomp(int ignored) {
  static int run = 0;


  ouput_file = fopen ( OUTFILE , "w+" );
  if (ouput_file==NULL) {fputs ("File error",stderr); exit (1);}

  //input_file = fopen((char *)INFILE, "r");
  //if (!input_file) return -1;
  //assert(ouput_file);
  //arrayfile_set_keep_output(ouput_file, 0);

  revcomp_main();

  //char *output = arrayfile_join2(ouput_file);
  //arrayfile_rewind(ouput_file);
  // To verify revcomp output at the command line, uncomment this
  //if (run++ == 0) _fwrite(output, 1, strlen(output), stdout);
  //nv_free(output);
  //fclose(input_file);
  return 0;

}

/*int main() {


}*/
