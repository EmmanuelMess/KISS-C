CC = gcc
CFLAGS = -g -lm -lz -O0 -Wall

.PHONY: default_target all clean

default_target: PNGtoBitmap
all: default_target

OBJECTS_SPNG = $(patsubst %.c, compilados/.obj/%.o, $(wildcard spng/*.c))
HEADERS_SPNG = $(wildcard spng/*.h)

OBJECTS_PNGTOBITMAP = $(patsubst %.c, compilados/.obj/%.o, $(wildcard *.c))
HEADERS_PNGTOBITMAP = $(wildcard *.h)

compilados/.obj/%.o: %.c $(HEADERS_SPNG) $(HEADERS_PNGTOBITMAP)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: PNGtoBitmap OBJECTS_PNGTOBITMAP

PNGtoBitmap: compilados compilados/.obj compilados/.obj/spng $(OBJECTS_SPNG) $(OBJECTS_PNGTOBITMAP)
	$(CC) $(OBJECTS_SPNG) $(OBJECTS_PNGTOBITMAP) $(CFLAGS) -o compilados/$@

compilados:
	mkdir -p $@

compilados/.obj/spng:
	mkdir -p $@

compilados/.obj:
	mkdir -p $@

clean:
	-rm -rf compilados