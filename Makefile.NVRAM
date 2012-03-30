all: lib benchmark64

LIBPATH=-L"/home/hendrix/browser_startup_time_ln/nonbrowser_mode"


benchmark64: benchstringio.o IOtimer.o fasta_10k_ref_output.o hash_map.o nv_map.o oswego_malloc.o nvmalloc_wrap.o fasta_nvram.o revcomp_nvram.o knucleotide_file.o procstat.o richards.o
	g++ -g bench-framework.c -o bench-framework IOtimer.o benchstringio.o fasta_10k_ref_output.o hash_map.o nv_map.o nvmalloc_wrap.o fasta_nvram.o  \
	revcomp_nvram.o knucleotide_file.o procstat.o richards.o ${LIBPATH}  -loswego  -lm -lpthread

IOtimer.o:IOtimer.c
	gcc -c -g IOtimer.c
	gcc mmap_read_multi_page_test.c -o mmap_read_multi_page_test

benchstringio.o: benchstringio.c
	gcc -c benchstringio.c -std=gnu99 -lm

fasta_10k_ref_output.o:fasta_10k_ref_output.c
	gcc -c -g fasta_10k_ref_output.c -std=gnu99
hash_map.o : hash_map.cc
	g++ -c -g hash_map.cc

nvmalloc_wrap.o: nvmalloc_wrap.c
	gcc -c -g nvmalloc_wrap.c ${LIBPATH} -loswego  -std=gnu99

revcomp_nvram.o: revcomp_nvram.c
	gcc -c -g revcomp_nvram.c -std=gnu99 -lpthread

procstat.o: procstat.c
	gcc -c -g procstat.c -std=gnu99 

fasta_nvram.o: fasta_nvram.c
	gcc -c -g fasta_nvram.c -std=gnu99

#deltablue.o: deltablue.c
#	 gcc -c -g deltablue.c nvmalloc_wrap.o -std=gnu99

nv_map.o: nv_map.c
	gcc -c -g nv_map.c 

richards.o: richards.c 
	gcc -c -g richards.c nvmalloc_wrap.o -std=gnu99

lib: oswego_malloc.c nv_map.o
	#gcc -c -g oswego_malloc.c
	gcc -Wall -fPIC -c oswego_malloc.c nv_map.o
	gcc -shared -Wl,-soname,liboswego.so.1 -o liboswego.so.1.0   oswego_malloc.o
	cp liboswego.so.1.0 liboswego.so
	cp liboswego.so.1.0 liboswego.so.1
	sudo cp liboswego.so* /usr/lib

knucleotide_file.o: knucleotide_file.c
	gcc -c -g knucleotide_file.c -std=gnu99

clean:
	rm -rf *.o
	rm -rf benchmark64
	rm -rf bench-framework
	rm -rf *.a
	rm -rf *.so
	rm -rf *.so.*
	rm -rf fasta_output
	rm -rf revcomp_output	
	 sudo rm -rf /usr/lib/liboswego*



