
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>
#include "vma.h"

list_t *create_list(unsigned int data_size)
{
	list_t *list = malloc(sizeof(list_t));
	if (!list) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}
	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
	return list;
}

node_t*
get_node(list_t *list, unsigned int n)
{
	if (!list || !list->head)
		return NULL;
	node_t *p;
	p = list->head;
	for (unsigned int i = 0; i < n; i++)
		p = p->next;
	return p;
}

void
add_node(list_t *list, unsigned int n)
{
	node_t *new_node = (node_t *)malloc(sizeof(node_t));
	if (!new_node) {
		fprintf(stderr, "Malloc failed\n");
		return;
	}
	new_node->next = NULL;
	new_node->prev = NULL;

	if (list->size == 0) {//lista e goala
		new_node->next = new_node;
		new_node->prev = new_node;
		list->head = new_node;
		list->size++;
	} else if (n == 0) {//adugare pe pozitia 0
		new_node->next = list->head;
		new_node->prev = list->head->prev;
		list->head->prev->next = new_node;
		list->head->prev = new_node;
		list->head = new_node;

		list->size++;
	} else {
		node_t *node;
		node = list->head;

		for (unsigned int i = 0; i < n - 1; i++)
			node = node->next;

		new_node->next = node->next;

		node_t *node_next;
		node_next = node->next;
		node_next->prev = new_node;
		node->next = new_node;
		new_node->prev = node;
		list->size++;
	}
}

node_t
*remove_node(list_t *list, unsigned int n)
{
	node_t *remove;
	if (list->size == 0)
		return NULL;

	if (n == 0) {
		remove = list->head;
		list->head = list->head->next;
		list->size--;
	} else {
		node_t *node;
		node = list->head;

		for (unsigned int i = 0; i < n - 1; i++)
			node = node->next;

		remove = node->next;
		remove->next->prev = remove->prev;
		remove->prev->next = remove->next;

		list->size--;
	}

	return remove;
}

void free_list(list_t **list)
{
	if (!list)
		return;
	while ((*list)->size != 0) {
		node_t *node = remove_node(*list, 0);
		free(node->data);
		free(node);
	}
	free(*list);
}

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena;
	arena = malloc(sizeof(arena_t));
	if (!arena) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}
	arena->arena_size = size;
	arena->alloc_list = create_list(sizeof(block_t));
	return arena;
}

//functie de creare a unui miniblock
miniblock_t
*create_miniblock(const uint64_t address, const uint64_t size, uint8_t perm)
{
	miniblock_t *miniblock =
	calloc(sizeof(miniblock_t), size);

	if (!miniblock) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}

	miniblock->start_address = address;
	miniblock->size = size;
	miniblock->perm = perm;
	return miniblock;
}

//functie de adaugare a unui miniblock pe pozitia p
//miniblock-ul se adauga pentru un block corespunzator
void
mini_m(const uint64_t adr, const uint64_t sz, int p, node_t *n)
{
	miniblock_t *miniblock = create_miniblock(adr, sz, 6);
	block_t *current_block = n->data;
	list_t *list_miniblock = current_block->miniblock_list;

	add_node(list_miniblock, p);
	node_t *mininode = get_node(list_miniblock, p);
	mininode->data = miniblock;
	current_block->size += sz;
	if (p == 0)
		current_block->start_address = adr;
}

//functie care adauga un nou block, pe o pozitie data
void
create_block(arena_t *arena, const uint64_t address, const uint64_t size, int p)
{
	list_t *list_miniblock = create_list(sizeof(miniblock_t));

	if (!list_miniblock) {
		fprintf(stderr, "Malloc failed\n");
		return;
	}

	add_node(list_miniblock, 0);
	//block-ul va avea un miniblock initial, pe pozitia 0
	miniblock_t *miniblock = create_miniblock(address, size, 6);

	node_t *mininode = get_node(list_miniblock, 0);

	mininode->data = miniblock;

	list_miniblock->head = mininode;

	block_t *block = malloc(sizeof(block_t));

	if (!block) {
		fprintf(stderr, "Malloc failed\n");
		return;
	}

	block->start_address = address;

	block->size = size;

	block->miniblock_list = list_miniblock;

	add_node(arena->alloc_list, p);

	node_t *node;

	node = get_node(arena->alloc_list, p);

	node->data = block;
}

//functie care adauga un block nou
//nu este creat niciun miniblock
void
block_free(arena_t *arena, const uint64_t address, const uint64_t size, int p)
{
	list_t *list_miniblock = create_list(sizeof(miniblock_t));

	if (!list_miniblock) {
		fprintf(stderr, "Malloc failed\n");
		return;
	}

	block_t *block = malloc(sizeof(block_t));

	if (!block) {
		fprintf(stderr, "Malloc failed\n");
		return;
	}

	block->start_address = address;

	block->size = size;

	block->miniblock_list = list_miniblock;

	add_node(arena->alloc_list, p);

	node_t *node;

	node = get_node(arena->alloc_list, p);

	node->data = block;
}

void concat_list(list_t *list1, list_t *list2)
{
	node_t *mini = get_node(list1, list1->size - 1);
	node_t *mini2 = get_node(list2, 0);
	mini->next = mini2;
	mini->next->prev = mini;
}

//functie care verifica daca o adresa este valida
void
valid(const uint64_t a, const uint64_t sz, block_t *cr, block_t *n)
{
	if (a > cr->start_address && a < cr->start_address + cr->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a == cr->start_address && a + sz > cr->start_address + cr->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a == cr->start_address + cr->size && a + sz > n->start_address) {
		printf("This zone was already allocated.\n");
		return;
	}

	if (a > cr->start_address && a < cr->start_address + cr->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a > n->start_address && a < n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}

	if (a == n->start_address && a + sz > n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
		}
	if (n->start_address < a + sz && a + sz < n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (cr->start_address < a + sz && a + sz < cr->start_address + cr->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < n->start_address && a + sz > n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < cr->start_address && a + sz > cr->start_address + cr->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < cr->start_address && a + sz > cr->start_address) {
		printf("This zone was already allocated.\n");
		return;
	}

	if (a < n->start_address && a + sz > n->start_address) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a >= n->start_address && a + sz < n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a + sz > n->start_address && a + sz < n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < n->start_address + n->size && a + sz > n->start_address + n->size) {
		printf("This zone was already allocated.\n");
		return;
	}
}

void
address_is_ok(const uint64_t a, const uint64_t s, block_t *c)
{
	if (a < c->start_address && a + s > c->start_address + c->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < c->start_address && a + s > c->start_address) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a == c->start_address && a + s == c->start_address + c->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a < c->start_address + c->size && a + s >  c->start_address + c->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (c->start_address < a && a + s <= c->start_address + c->size) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a == c->start_address) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (a > c->start_address && a < c->start_address + c->size) {
		printf("This zone was already allocated.\n");
		return;
	}
}

//se verifica initial daca lista din arena este goala
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n"); return;
	} else if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n"); return; }
	if (arena->alloc_list->size  == 0) {//adaugare block initial
		create_block(arena, address, size, 0);
	} else if (arena->alloc_list->size == 1) {//avem un block in lista
		node_t *current_node = arena->alloc_list->head;
		block_t *current_block = current_node->data;
		uint64_t cur_total = current_block->start_address + current_block->size;
		address_is_ok(address, size, current_block);
		if (address + size < current_block->start_address) {
			create_block(arena, address, size, 0);//adugare block stanga
		} else if (cur_total < address) {
			create_block(arena, address, size, 1);//adugare block dreapta
		} else if (address + size == current_block->start_address) {
			mini_m(address, size, 0, current_node);//adugare miniblock stanga
		} else if (cur_total == address) {//augare miniblock dreapta
			list_t *list = current_block->miniblock_list;
			mini_m(address, size, list->size, current_node); }
	} else if (arena->alloc_list->size > 1) {//2 sau mai multe block-uri
		unsigned int i = 1;
		while (i < arena->alloc_list->size) {
			node_t *current_node = get_node(arena->alloc_list, i - 1);
			node_t *next_node = get_node(arena->alloc_list, i);
			block_t *current_block = current_node->data;
			block_t *next_block = next_node->data;
			uint64_t tot = current_block->start_address + current_block->size;
			uint64_t tot_n = next_block->start_address + next_block->size;
			unsigned int total = address + size;
			if (address > current_block->start_address && address < tot) {
				printf("This zone was already allocated.\n"); return; }
			if (address == current_block->start_address && total > tot) {
				printf("This zone was already allocated.\n"); return; }
			if (address > next_block->start_address && address < tot_n) {
				printf("This zone was already allocated.\n"); return; }
			if (current_block->start_address < total && total < tot) {
				printf("This zone was already allocated.\n"); return; }
			if (address < current_block->start_address && total > tot) {
				printf("This zone was already allocated.\n"); return; }
			if (total > next_block->start_address && total < tot_n) {
				printf("This zone was already allocated.\n"); return; }
			uint64_t sum = current_block->start_address + current_block->size;
		if (i == 1) {//adugare inintea tuturor block-urilor din lista
			if (total < current_block->start_address)
				create_block(arena, address, size, 0);
			else if (total == current_block->start_address)
				mini_m(address, size, 0, current_node); }
		if (sum < address && total < next_block->start_address) {//intre blocuri
			create_block(arena, address, size, i);
		} else if ((sum == address) && (total == next_block->start_address)) {
			list_t *list_mini = ((block_t *)current_node->data)->miniblock_list;
			list_t *list_mini_n = ((block_t *)next_node->data)->miniblock_list;
			mini_m(address, size, list_mini->size, current_node);
			node_t *mini = get_node(list_mini, list_mini->size - 1);
			node_t *mini2 = get_node(list_mini_n, 0);
			mini->next = mini2; mini2->prev = mini;
			list_mini->size += list_mini_n->size;
			current_block->size += next_block->size;
			node_t *aux = remove_node(arena->alloc_list, i);
			free(((block_t *)aux->data)->miniblock_list); free(aux->data);
			free(aux);//se formeaza 3 miniblock-uri, un block este eliberat
		} else if (sum == address && address < next_block->start_address) {
			list_t *list_mini = ((block_t *)current_node->data)->miniblock_list;
			mini_m(address, size, list_mini->size, current_node);
		} else if (sum < address && total == next_block->start_address) {
			mini_m(address, size, 0, next_node); }
		i++;
		if (i == arena->alloc_list->size) {//adugare block la final
			node_t *node = get_node(arena->alloc_list, i - 1);
			block_t *block = node->data;
			if (block->start_address + block->size < address) {
				create_block(arena, address, size, i);
			} else if (block->start_address + block->size == address) {
				list_t *list_miniblock;
				list_miniblock = ((block_t *)node->data)->miniblock_list;
				mini_m(address, size, list_miniblock->size, node); } }
		}
	}
}

void dealloc_arena(arena_t *arena)
{
	node_t *current_node;
	block_t *current_block;

	current_node = arena->alloc_list->head;
	//se elibereaza memoria pentru fiecare block si miniblock in parte
	//pentru listele create si pentru noduri
	while (arena->alloc_list->size != 0) {
		current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;
		while (list_miniblock->size != 0) {
			node_t *mininode = list_miniblock->head;
			if (((miniblock_t *)list_miniblock->head->data)->rw_buffer)
				free(((miniblock_t *)list_miniblock->head->data)->rw_buffer);
			node_t *miniaux = get_node(list_miniblock, 1);
			free(mininode->data);
			mininode = remove_node(list_miniblock, 0);
			free(mininode);
			mininode = miniaux;
			}
		free(list_miniblock);
		node_t *node_auux = get_node(arena->alloc_list, 1);
		free(current_node->data);
		current_node = remove_node(arena->alloc_list, 0);
		free(current_node);
		current_node = node_auux;
	}
	free(arena->alloc_list);
	free(arena);
}

void free_block(arena_t *arena, const uint64_t address)
{
	list_t *list_block = arena->alloc_list;
	unsigned int i, j;
	for (j = 0; j < list_block->size; j++) {
		node_t *current_node = get_node(arena->alloc_list, j);
		block_t *current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;
		for (i = 0; i < list_miniblock->size; i++) {
			node_t *mininode = get_node(list_miniblock, i);
			miniblock_t *miniblock = mininode->data;
			uint64_t address_current = miniblock->start_address;
			if (address_current == address) {//am gasit adresa
				if (list_miniblock->size == 1) {//avem un singur miniblock
					free(mininode->data);
					mininode = remove_node(list_miniblock, 0);
					free(mininode);
					free(list_miniblock);
					free(current_node->data);//se elibereaza block-ul
					current_node = remove_node(arena->alloc_list, i);
					free(current_node);
					return;
				}  else if (mininode == list_miniblock->head) {
					node_t *mininode_next = mininode->next;
					miniblock_t *miniblock_next = mininode_next->data;
					current_block->size -= miniblock->size;
				//eliberare miniblock de la inceput
				uint64_t address_new = miniblock_next->start_address;
				current_block->start_address = address_new;
				free(mininode->data);
				mininode = remove_node(list_miniblock, i);
				free(mininode);
				return;
				} else {
					node_t *last;
					last = get_node(list_miniblock, list_miniblock->size - 1);
					if (mininode == last) {//eliberare ultimul miniblock
						current_block->size -= miniblock->size;
						free(mininode->data);
						mininode = remove_node(list_miniblock, i);
						free(mininode);
						//return;
					} else {//se va crea un nou block cu miniblock-urile de dupa
						node_t *mini_n = get_node(list_miniblock, i + 1);
						uint64_t ad_start;
						ad_start = ((miniblock_t *)mini_n->data)->start_address;
						unsigned int new_sz = 0;
					for (unsigned int k = 0; k < i; k++) {
						node_t *mininode_aux = get_node(list_miniblock, k);
						miniblock_t *miniblock_aux = mininode_aux->data;
						new_sz += miniblock_aux->size;//noua dimensiune
					}
						uint64_t b_sz;
						b_sz = current_block->size  - new_sz - miniblock->size;
						block_free(arena, ad_start, b_sz, j + 1);//block nou
						node_t *tmp_node = get_node(arena->alloc_list, j + 1);
						block_t *block = tmp_node->data;
						list_t *newl = block->miniblock_list;
						newl->size = list_miniblock->size - 1 - i;
						newl->head = mini_n;
						newl->head->prev = NULL;
						mininode->next->prev = NULL;
						current_block->size = new_sz;
						uint64_t new_value;
						new_value = list_miniblock->size - newl->size - 1;
						list_miniblock->size = new_value;//noua dimeniune listei
						free(mininode->data);
						free(mininode);
						return;
					}
				}
		}
	}
}

printf("Invalid address for free.\n");
		return;
}

//fuctie necesara pentru permisiunile unui miniblock
char *per(char perm)
{
	if (perm == 7)
		return "RWX";
	else if (perm == 6)
		return "RW-";
	else if (perm == 5)
		return "R-X";
	else if (perm == 4)
		return "R--";
	else if (perm == 3)
		return "-WX";
	else if (perm == 2)
		return "-W-";
	else if (perm == 1)
		return "--X";
	else if (perm == 0)
		return "---";
	else
		return 0;
}

void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	uint64_t size_new = arena->arena_size;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		node_t *current_node = get_node(arena->alloc_list, i);
		block_t *current_block = current_node->data;
		size_new -= current_block->size;//scadem memoria ocupata
	}
	printf("Free memory: 0x%lX bytes\n", size_new);
	printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
	int number = 0;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		node_t *current_node = get_node(arena->alloc_list, i);
		block_t *current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;
		number += list_miniblock->size;
	}
	printf("Number of allocated miniblocks: %d\n", number);
	//se parcurg toate block-urile si miniblock-urile
	//pentru fiecare se afiseaza informatiile cerute
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		node_t *current_node = get_node(arena->alloc_list, i);
		block_t *current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;
		printf("\nBlock %d begin\n", i + 1);
		uint64_t total;
		total = current_block->start_address + current_block->size;
		printf("Zone: 0x%lX - 0x%lX\n", current_block->start_address, total);
		for (unsigned int j = 0; j < list_miniblock->size; j++) {
			node_t *mininode = get_node(list_miniblock, j);
			miniblock_t *mini = mininode->data;
			uint64_t all = mini->start_address + mini->size;
			uint64_t start = mini->start_address;
			printf("Miniblock %d", j + 1);
			printf(":\t\t0x%lX\t\t-\t\t", start);
			printf("0x%lX\t\t| %s\n", all, per(mini->perm));
		}
		printf("Block %d end\n", i + 1);
	}
}

void
write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	uint64_t sizec = size;
	for (unsigned int j = 0; j < arena->alloc_list->size; j++) {
		node_t *current_node = get_node(arena->alloc_list, j);
		block_t *current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;
		uint64_t address_total;
		address_total = current_block->start_address + current_block->size;
		uint64_t start_curr = current_block->start_address;
		if (start_curr <= address && address <= address_total) {
			if (address + size > address_total) {
				sizec = address_total - address;
				printf("Warning: size was bigger than the block size. ");
				printf("Writing %ld characters.\n", sizec);
			}
			for (unsigned int i = 0; i < list_miniblock->size; i++) {
				node_t *mininode = get_node(list_miniblock, i);
				miniblock_t *miniblock = mininode->data;
				uint8_t perm = miniblock->perm;
				uint64_t total = miniblock->start_address + miniblock->size;
				uint64_t start = miniblock->start_address;
				if (perm == 5 || perm == 4 || perm == 1 || perm == 0) {
					printf("Invalid permissions for write.\n");
					return;//verificam daca avem permisiuni de scriere
				}
				miniblock->rw_buffer = NULL;
				char *buffer = malloc(miniblock->size * sizeof(char));
				miniblock->rw_buffer = buffer;
				uint64_t dif = address - miniblock->start_address;
				if (start == address && address + sizec == total) {
					memcpy(miniblock->rw_buffer, data, sizec);
					return;//scriem intr-un singur miniblock de la inceputul lui
				}
				if (start  <= address && address + sizec <= total) {
					memcpy(miniblock->rw_buffer + dif, data, sizec);
					return;//scriem intr-un singur miniblock de la o adresa data
				} else if (start <= address && address + sizec > total) {
					uint64_t new_size = miniblock->size - dif;
					memcpy(miniblock->rw_buffer + dif, data, new_size);
					sizec -= new_size;//scriem in mai multe miniblock-uri
				while (sizec != 0 || mininode->next) {
					node_t *mini_next = mininode->next;
					miniblock_t *miniblock_next = mini_next->data;
					uint8_t p = miniblock_next->perm;
					if (p == 5 || p == 4 || p == 1 || p == 0) {
						printf("Invalid permissions for write.\n");
						return;
					}
					miniblock_next->rw_buffer = NULL;
					char *buffer = malloc(miniblock_next->size * sizeof(char));
					miniblock_next->rw_buffer = buffer;

					if (miniblock_next->size >= sizec) {
						uint64_t nr = sizec;
						memcpy(miniblock_next->rw_buffer, data + new_size, nr);

						sizec = 0;//am terminat de scris
						return;

					} else {
						uint64_t siz = miniblock_next->size;
						memcpy(miniblock_next->rw_buffer, data + new_size, siz);

						new_size += miniblock_next->size;//tinem cont cat scriem

						sizec -= miniblock_next->size;
					}
					mininode = mininode->next;
					}
					}
		}
	}
}

printf("Invalid address for write.\n");
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	uint64_t sizec = size;
	for (unsigned int j = 0; j < arena->alloc_list->size; j++) {
		node_t *curr_node = get_node(arena->alloc_list, j);
		block_t *curr_block = curr_node->data;
		list_t *list_miniblock = curr_block->miniblock_list;
		for (unsigned int i = 0; i < list_miniblock->size; i++) {
			node_t *mininode = get_node(list_miniblock, i);
			miniblock_t *miniblock = mininode->data;
			if (miniblock->perm == 3 || miniblock->perm == 2 ||
				miniblock->perm == 1 || miniblock->perm == 0) {
				printf("Invalid permissions for read.\n");
				return;//verificam daca avem permisiuni de citire
			}
		}
	}
	for (unsigned int j = 0; j < arena->alloc_list->size; j++) {
		node_t *curr_node = get_node(arena->alloc_list, j);
		block_t *curr_block = curr_node->data;
		list_t *list_miniblock = curr_block->miniblock_list;
		uint64_t address_total = curr_block->start_address + curr_block->size;
		uint64_t str = curr_block->start_address;
		if (str <= address && address < address_total) {
			if (address + size > address_total) {
				sizec = address_total - address;
				printf("Warning: size was bigger than the block size.");
				printf(" Reading %ld characters.\n", sizec);
			}
		for (unsigned int i = 0; i < list_miniblock->size; i++) {
			node_t *mininode = get_node(list_miniblock, i);
			miniblock_t *miniblock = mininode->data;
			uint64_t tot = miniblock->start_address + miniblock->size;
			uint8_t start = miniblock->start_address;
			uint8_t p = miniblock->perm;
			if (p == 3 || p == 2 || p == 1 || p == 0) {
				printf("Invalid permissions for read.\n");
				return;
			}
			uint64_t dif = address - miniblock->start_address;
			if (miniblock->start_address == address && address + sizec == tot) {
				for (unsigned int i = 0; i < sizec; i++)
					printf("%c", ((char *)miniblock->rw_buffer)[i]);
				printf("\n");
				return;//citim dintr-un singur miniblock
			}
			if (miniblock->start_address  <= address && address + sizec < tot) {
				int i = 0;
				while (sizec > 0) {
					printf("%c", ((char *)miniblock->rw_buffer)[i + dif]);
					i++, sizec--;
					}
				printf("\n");
				return;//citim dintr-un singur miniblock, de la o adresa data
			} else if (start <= address && address + sizec > tot) {
				uint64_t new_size = miniblock->size - dif;//int i = 0;
				sizec -= new_size;
				for (unsigned int i = 0; i < new_size; i++)
					printf("%c", ((char *)miniblock->rw_buffer)[dif + i]);
				while (sizec != 0 || !mininode->next) {//citim din mai multe
					node_t *mini_next = mininode->next;
					miniblock_t *n = mini_next->data;
					uint8_t per = n->perm;
				if (per == 3 || per == 2 || per == 1 || per == 0) {
					printf("Invalid permissions for read.\n");
					return;
				}
				if (n->size >= sizec) {
					for (unsigned int i = 0; i < sizec; i++)
						printf("%c", ((char *)n->rw_buffer)[i]);
					sizec = 0;//terminam de citit
					printf("\n"); return;
				} else {
					for (unsigned int i = 0; i < n->size; i++)
						printf("%c", ((char *)n->rw_buffer)[i]);
					sizec -= n->size;
				}
					mininode = mininode->next; }
			}
				}
		}
}

printf("Invalid address for read.\n");
}

void mprotect(arena_t *arena, uint64_t address, char *permission)
{
	char line[150];
	fgets(line, 150, stdin);

	int ok  = 0;

	for (unsigned int j = 0; j < arena->alloc_list->size; j++) {
		node_t *current_node = get_node(arena->alloc_list, j);
		block_t *current_block = current_node->data;
		list_t *list_miniblock = current_block->miniblock_list;

		uint64_t address_total =
		current_block->start_address + current_block->size;
		uint64_t start = current_block->start_address;
		if (start <= address && address < address_total) {
			for (unsigned int i = 0; i < list_miniblock->size; i++) {
				node_t *mininode = get_node(list_miniblock, i);
				miniblock_t *miniblock = mininode->data;
				if (miniblock->start_address == address) {
					permission = strtok(line, " |\n");//luam fiecare parametru
					ok = 1;
					int perm = 0;
				while (permission) {
					if (strcmp(permission, "PROT_NONE") == 0)
						perm = perm + 0;
					else if (strcmp(permission, "PROT_READ") == 0)
						perm = perm + 4;//permisiune de citire
					else if (strcmp(permission, "PROT_WRITE") == 0)
						perm = perm + 2;//permisiune de scriere
					else if (strcmp(permission, "PROT_EXEC") == 0)
						perm = perm + 1;//permisiune de executie
					permission = strtok(NULL, " |\n");
					}
					miniblock->perm = perm;//atribuim noua permisiune
				}
			}
		}
	}
	if (!ok)
		printf("Invalid address for mprotect.\n");
}
