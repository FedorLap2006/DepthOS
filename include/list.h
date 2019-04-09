#include <types.h>


/////////////// DOUBLY LINKED LIST ///////////////

typedef struct  __dliste {
	void* data;
	
	uint16_t datae_size;

	struct __dliste *prev;
	struct __dliste *next;
}dliste_t;

typedef struct __dlist {
	struct __dliste *begin;
	struct __dliste *cur;
	struct __dliste *end;
}dlist_t;

void __dlist_add(struct __dliste *newe, struct __dlist *head);

void __dlist_delIndex(size_t index,struct __dlist *head);

void __dlist_delAddr(struct __dliste *entry);

struct __dliste* __dlist_getIndex(size_t index);



///////////// SIMPLY LINKED LIST ////////////////

typedef struct __sliste {
	void *data;
	uint16_t datae_size;
	struct __sliste *next;
}sliste_t;

typedef struct __slist {
	struct __sliste *begin;
	struct __sliste *cur;
	struct __sliste *end;
}slist_t;


void __slist_add(struct __sliste *newe,struct __slist *head);

void __slist_delIndex(size_t index,struct __slist *head);

void __slist_delAddr(struct __sliste *entry);

struct __sliste* __slist_getIndex(size_t index);


