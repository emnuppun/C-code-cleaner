#include <stdio.h>
#include <stdlib.h>

#include "memory.h"


/*Setting memory address to memory tracker*/
M_tracker* set_memory(M_tracker *mem, void *add) {
    mem->count++;
    mem->list = realloc(mem->list, mem->count * sizeof(M_unit));
    M_unit tmp = {add, NOT_FREED};
    mem->list[mem->count - 1] = tmp;
    return mem;
}

/*Freeing one memory address from memory tracker*/
void free_memory(M_tracker *mem, void *ptr) {
	for(int i = 0; i < mem->count; i++) {
		if (mem->list[i].ad == ptr && mem->list[i].status == NOT_FREED) {
			free(mem->list[i].ad);
			mem->list[i].status = FREED;
		}
	}
}

/*Freeing all the memory including memory tracker itself and closing all the file pointers*/
void release_memory(M_tracker *mem) {
	for(int i = 0; i < mem->count; i++) {
		if (mem->list[i].status == NOT_FREED) {
			free(mem->list[i].ad);
			mem->list[i].status = FREED;
		}
	}
	free(mem->list);
	for(int i = 0; i < mem->fp_count; i++) {
		if (mem->fp_list[i].status == NOT_FREED) {
			fclose(mem->fp_list[i].ad);
			mem->fp_list[i].status = FREED;
		}
	}
	free(mem->fp_list);
	free(mem);
}

/*Setting file pointer to memory tracker*/
M_tracker* set_fp(M_tracker *mem, void *add)
{
    mem->fp_count++;
    mem->fp_list = realloc(mem->fp_list, mem->fp_count * sizeof(M_unit));
    M_unit tmp = {add, NOT_FREED};
    mem->fp_list[mem->fp_count - 1] = tmp;
    return mem;
}

/*Closing one file pointer*/
void close_fp(M_tracker *mem, void *fp)
{
	for(int i = 0; i < mem->fp_count; i++) {
		//mem->fp_list[i].ad = fp;
		if (mem->fp_list[i].ad == fp && mem->fp_list[i].status == NOT_FREED) {
			fclose(mem->fp_list[i].ad);
			mem->fp_list[i].status = FREED;
		}
	}
}