CC=gcc
CFLAGS=-I.
DEPS = get_time.h global_api.h global_def.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bed: bed.o global_api.o
	gcc -o bed bed.o global_api.o -I.
