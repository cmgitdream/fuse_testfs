
fuse_hl: fuse_hl.o
	gcc -o fuse_hl fuse_hl.o -lfuse
fuse_hl.o: fuse_hl.c
	gcc -g -D_FILE_OFFSET_BITS=64 -c fuse_hl.c

clean:
	rm -rf *.o fuse_hl
