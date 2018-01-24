#ifndef _MEMORY_H_
#define _MEMORY_H_

enum m_status {FREED, NOT_FREED};

typedef struct memory_unit
{
	void *ad;
	enum m_status status;

} M_unit;

/*Memory tracker to keep track of the memory addresses. 
Helps with signal handling so there's no memory leaks.
Fp_list includes all the file pointers because they need
to be handled differently than other memory addresses 
that my program uses.*/
typedef struct memory_tracker
{
	int count;
	int fp_count;
	M_unit *list;
	M_unit *fp_list;
} M_tracker;

void release_memory(M_tracker *mem);

void free_memory(M_tracker *mem, void *ptr);

M_tracker* set_memory(M_tracker *mem, void *add);

M_tracker* set_fp(M_tracker *mem, void *add);

void close_fp(M_tracker *mem, void *ptr);

#endif