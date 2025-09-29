CC = gcc
CFLAGS = -Wall -g `pkg-config fuse --cflags`

LDLIBS = `pkg-config fuse --libs`

all: fusefs test

fusefs: fusefs.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

test: test.c
	$(CC) -o $@ $<

read_benchmark: read_benchmark.c
	$(CC) -o $@ $<

clean:
	rm -f fusefs test read_benchmark
	rm -rf /tmp/fuse_cache/*

mount: fusefs
	mkdir -p ~/mount_point
	./fusefs krisub@c220g5-111307.wisc.cloudlab.us ~/mount_point -f

.PHONY: all clean mount