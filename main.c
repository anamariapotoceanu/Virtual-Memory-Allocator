
#include "vma.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>

int main(void)
{
	char command[50];
	arena_t *arena;
	int ok = 1;
	// char *permission;
	while (ok) {
		scanf("%s", command);
		if (strcmp(command, "ALLOC_ARENA") == 0) {
			uint64_t size;
			scanf("%ld", &size);
			arena = alloc_arena(size);
		} else if (strcmp(command, "ALLOC_BLOCK") == 0) {
			uint64_t address, size;
			scanf("%ld%ld", &address, &size);
			alloc_block(arena, address, size);
		} else if (strcmp(command, "FREE_BLOCK") == 0) {
			uint64_t address;
			scanf("%ld", &address);
			free_block(arena, address);
		} else if (strcmp(command, "PMAP") == 0) {
			pmap(arena);
		} else if (strcmp(command, "WRITE") == 0) {
			uint64_t address, size;
			int8_t *data;
			scanf("%ld%ld", &address, &size);
			data = malloc((size + 1) * sizeof(char));
			unsigned int i;
			getchar();
			for (i = 0; i < size; i++)
				data[i] = getchar();
			write(arena, address, size, data);
			free(data);
		} else if (strcmp(command, "READ") == 0) {
			uint64_t address, size;
			scanf("%ld%ld", &address, &size);
			read(arena, address, size);
		} else if (strcmp(command, "READ_BLOCK") == 0) {
			uint64_t address, size;
			scanf("%ld%ld", &address, &size);
		} else if (strcmp(command, "MPROTECT") == 0) {
			uint64_t address;
			char *permission = malloc(sizeof(char));
			scanf("%ld", &address);
			mprotect(arena, address, permission);
			free(permission);
		} else if (strcmp(command, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			ok = 0;
		} else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
