
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
typedef struct node_t node_t;
typedef struct list_t list_t;
typedef struct block_t block_t;
typedef struct miniblock_t miniblock_t;
typedef struct arena_t arena_t;

struct node_t {
node_t *next;
node_t *prev;
void *data;
};

struct list_t {
node_t *head;
unsigned int data_size;
unsigned int size;
};

struct block_t {
uint64_t start_address;
size_t size;
void *miniblock_list;
};

struct miniblock_t {
uint64_t start_address;
size_t size;
uint8_t perm;
void *rw_buffer;
};

struct arena_t {
uint64_t arena_size;
list_t *alloc_list;
};

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write
(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, char *permission);
