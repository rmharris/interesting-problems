CC = cc

rotate: rotate.c
	$(CC) -o rotate rotate.c

clean:
	rm -f rotate tests/*.actual

test:
	cd tests ; ./test.sh
