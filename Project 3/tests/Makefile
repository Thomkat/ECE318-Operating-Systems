all: gather_stats

mem_syscalls_wrapper.o: mem_syscalls_wrapper.c mem_syscalls_wrapper.h
	gcc -Wall -c mem_syscalls_wrapper.c -o mem_syscalls_wrapper.o

libmem_syscalls_wrapper.a: mem_syscalls_wrapper.o
	ar rcs libmem_syscalls_wrapper.a mem_syscalls_wrapper.o

gather_stats: mem_syscalls_wrapper.o libmem_syscalls_wrapper.a
	gcc gather_stats.c -o gather_stats -lmem_syscalls_wrapper -L.

clean:
	rm -f *.o *.a