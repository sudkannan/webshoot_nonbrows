#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "IOtimer.h"
#include "gtthread.h"
#include <proc/readproc.h>

//#define SPIN_LOCK

#ifdef SPIN_LOCK
#include "gtthread_spinlocks.h"
#endif

struct timeval startw, endw;
struct timeval startr, endr;

#ifdef SPIN_LOCK
struct gt_spinlock_t spinlock_r, spinlock_w;
#endif

int lock_created = 0;
long total_size;

static unsigned long total_write_size;
static unsigned long total_read_size ;
static unsigned long tot_mem_usage =0;
static unsigned long sample_cnt = 0;


pthread_mutex_t mutex_r = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_w = PTHREAD_MUTEX_INITIALIZER;

#ifdef SPIN_LOCK

void initialize_timer() {
	
	gt_spinlock_init(&spinlock_r);

}
#endif

void IOtimer_clear() {

#ifdef SPIN_LOCK
	initialize_timer();
#endif

	total_read_time = 0;
	total_write_time = 0;
	total_read_size = 0;
	total_write_size = 0;
	tot_mem_usage = 0;
	 get_proc_usage();
}

/* To calculate simulation time */
long simulation_time(struct timeval start, struct timeval end )
{
	long current_time;

	current_time = ((end.tv_sec * 1000000 + end.tv_usec) -
                	(start.tv_sec*1000000 + start.tv_usec));

	return current_time;
}


size_t _fread ( void * ptr, size_t size, size_t count, FILE * stream ){

	size_t  len = 0;

	 gettimeofday(&startr, NULL);

	 len = fread (ptr, size, count, stream );

	 gettimeofday(&endr, NULL);

	total_read_time += simulation_time(startr, endr );

	total_read_size = total_read_size + len;

	 get_proc_usage();

	return len;

}




char * _fgets( char * str, size_t num, FILE * stream ) {

	char *ptr = NULL;

#ifdef SPIN_LOCK
	gt_spin_lock(&spinlock_r);
#endif

	 gettimeofday(&startr, NULL);

 	 ptr = fgets(str, num, stream );

	 gettimeofday(&endr, NULL);
	
 	 total_read_time += simulation_time(startr, endr );
	
	 total_read_size = total_read_size + strlen(str);;

#ifdef SPIN_LOCK
     gt_spin_unlock(&spinlock_r);
#endif

	 get_proc_usage();
	

	return ptr;
}

int _fputs ( char * str, FILE * stream ) {


#ifdef SPIN_LOCK
  	 gt_spin_lock(&spinlock_w);
#endif

	 gettimeofday(&startw, NULL);

 	 fputs(str, stream );

	 gettimeofday(&endw, NULL);
	
 	 total_write_time += simulation_time(startw, endw );

	 //fprintf(stderr,"strlen(str) %u \n", strlen(str));

	 total_write_size = total_write_size + strlen(str);

#ifdef SPIN_LOCK
     gt_spin_unlock(&spinlock_w);
#endif

	  get_proc_usage();

 	 return 0;
}



size_t _fwrite ( void * ptr, size_t size, size_t count, FILE * stream ){

	
	size_t bytes = 0;
	
	 gettimeofday(&startw, NULL);

    bytes = fwrite (ptr, size, count, stream );

	 gettimeofday(&endw, NULL);

	 total_write_time += simulation_time(startw, endw );

	 total_write_size = total_write_size + size;

	  get_proc_usage();

	return bytes;		

}

ssize_t _write(int fd, void *ptr, size_t size){

     ssize_t bytes = 0;

	 gettimeofday(&startw, NULL);

	 bytes = write(fd, ptr, size); 	

	 gettimeofday(&endw, NULL);

	 total_write_time += simulation_time(startw, endw );

	 total_write_size = total_write_size + size;

	  get_proc_usage();
 
	  return bytes;

}


void* _malloc(size_t size) {

	void *ptr = NULL;

	ptr = malloc(size);

	get_proc_usage();

	return ptr;

}

void print_total_write_time(){
	
	unsigned long avg_mem_usg = 0;

	fprintf(stdout,"Write time %ld  \n",total_write_time);
	fprintf(stdout,"Read time %ld  \n",total_read_time);
	fprintf(stdout,"Write size %ld \n", total_write_size);
	fprintf(stdout,"Read size %ld \n", total_read_size);


	 fprintf(stdout,"avg_mem_usg %ld \n", tot_mem_usage);

}

int get_proc_usage() {
  /*struct proc_t usage;
  look_up_our_self(&usage);

  sample_cnt++;

  if(tot_mem_usage < usage.vsize)	
	  tot_mem_usage = usage.vsize;*/

}



