#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define FILEPATH "/tmp/mmapped3.bin"
#define NUMINTS  (90)
#define FILESIZE (NUMINTS * sizeof(int))

#define __NR_nv_mmap_pgoff     301 

#define MAP_SIZE 203333411 
#define SEEK_BYTES 1024*1024*1024

#define INVALID_INPUT -2;

 void * realloc_map (void *addr, size_t len, size_t old_size)
  {
          void *p;

          p = mremap (addr, old_size, len, MREMAP_MAYMOVE);
          return p;
  }

struct nvmap_arg_struct{

	unsigned long fd;
	unsigned long offset;
	int chunk_id;
	int proc_id;
	int pflags;
};


//Method to generate random data
int generate_random_text( char *addr, unsigned long len, unsigned long  num_words  ) {

    unsigned long idxa =0, idxb =0;
    unsigned long cntr =0;
    int wordsize;
    int maxwordsize = 0 ;
    int idxc =0;

    maxwordsize = len / num_words;

    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (idxa = 0; idxa < num_words; idxa++) {

        wordsize = rand()%  maxwordsize;

        for (idxb = 0; idxb < wordsize; idxb++) {

            if( cntr >= len) break;

            idxc = rand()% (sizeof(alphanum)/sizeof(alphanum[0]) - 1);
            addr[cntr] = alphanum[idxc];
            cntr++;
        }
        addr[cntr] = ' ';
        cntr++;
    }
    return cntr;

}

int str_cmp( char *addr1, char *addr2, size_t len) {

	size_t idx = 0;

	if(len < 1 || !addr1 || !addr2) {
		printf("invalud len or addr \n");
		return INVALID_INPUT;
	}

	while(idx < len) {

		if( *addr1 != *addr2) {		
			printf("string not equal: addr1 %c, addr2 %c \n", *addr1 , *addr2);
			return -1;
		}
		printf ("addr1 %c, addr2 %c \n", *addr1 , *addr2);
		addr1++;
		addr2++;
		idx++;
	}

	return 0;
}

int main(int argc, char *argv[])
{
    int i;
    int fd;
    int result;
    char *map, *map2, *map_read;  /* mmapped array of int's */
    void *start_addr;
	int count =0;  
    unsigned long offset = 0;
	int chunk_id = 1;
    int proc_id = 0;
	struct nvmap_arg_struct a;

	proc_id = atoi(argv[1]);
	if(!proc_id) {
		fprintf(stderr,"proc id is null \n");
		return -1;
	}

    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
	perror("Error opening file for writing");
	exit(EXIT_FAILURE);
    }

    result = lseek(fd, SEEK_BYTES, SEEK_SET);
    if (result == -1) {
	close(fd);
	perror("Error calling lseek() to 'stretch' the file");
	exit(EXIT_FAILURE);
    }
    
    result = write(fd, "", 1);
    if (result != 1) {
	close(fd);
	perror("Error writing last byte of the file");
	exit(EXIT_FAILURE);
    }


    i =0;

	a.fd = fd;
	a.offset = offset;
	a.chunk_id =chunk_id;
	a.proc_id = proc_id;
	a.pflags = 1;

	 {

	 printf("going to mmap readd \n");

	map_read = (char *) syscall(__NR_nv_mmap_pgoff, 0, MAP_SIZE,  PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, &a );
	//map_read = (char *)mmap( 0, MAP_SIZE,  PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (map_read == MAP_FAILED) {
	    close(fd);
    	perror("Error mmapping the file");
	    exit(EXIT_FAILURE);
    }

	//fprintf(stdout, "\n\n\n\n");

	//fprintf(stdout, "%s \n", map_read);

	i = 100 * 1024 * 1024;


	 while ( i < MAP_SIZE) {

		fprintf(stdout, "%c", map_read[i]);
		i++;
	}


  	if(str_cmp(map, map_read, MAP_SIZE)) {
		printf("multipage test failed \n");
	}else {
		printf("multipage test succeeded \n");
	}

	}

     close(fd);

    return 0;
}
