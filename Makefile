CC=gcc -g 
CFLAGS=-Wall -Wextra -std=c99

TARGETS=vma

build: $(TARGETS)


vma: main.o vma.o
	$(CC) $(CFLAGS) main.o vma.o -o vma
main.o: main.c vma.h
	$(CC) $(CFLAGS) -c main.c 
vma.o: vma.c
	$(CC) $(CFLAGS) -c vma.c

run_vma: vma
	./vma

clean:
	rm -f *.o $(TARGETS)

PHONY: pack clean