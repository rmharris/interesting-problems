# Copyright (c) 2018, Robert Harris.

CC = cc
LDFLAGS = -m64
CFLAGS = -I. -m64
OBJS = main.o sparse_array.o

wc++: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

sparse_array.o: sparse_array.c sparse_array.h sparse_array_impl.h
	$(CC) $(CFLAGS) -c -o $@ sparse_array.c

main.o: main.c sparse_array.h
	$(CC) $(CFLAGS) -c -o $@ main.c

clean:
	rm -f $(OBJS) wc++ tests/*.result

test:
	cd tests ; ./test.sh
