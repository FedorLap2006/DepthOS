#include <depthos/heap.h>
#include <depthos/paging.h>
#include <depthos/console.h>

extern uint32_t end;
uint32_t _memory_start = (uint32_t)&end;
extern page_t heap_blks_pgt1[1024] __align(4096);

typedef struct __heap_blockm_state {
	enum {
		CACHEB,
		RESB,
	}type;
	uint32_t begin;
	uint32_t end;
	union {
		struct {
			heap_cacheblock_t* cbtop;
			heap_cacheblock_t* cbroot;
		};
		struct {
			heap_resblock_t* rbtop;
			heap_resblock_t* rbroot;
		};
	};
}heap_blockm_state_t;



static heap_blockm_state_t _cbstate = { .type = CACHEB };
static heap_blockm_state_t _rbstate = { .type = RESB };
bool __heap_autorescue_mode;

static heap_cacheblock_t* __heap_find_cacheblock() {
	heap_cacheblock_t *cur;
	cur = _cbstate.cbroot;
	while(cur != NULL && cur->busy_mru) {
		cur = cur->next;
	}
	return cur;

}


static heap_cacheblock_t* __heap_alloc_cacheblock() {
	heap_cacheblock_t *newblk;
		if (_cbstate.cbtop == NULL) {
				_cbstate.cbtop = (heap_cacheblock_t*)_cbstate.begin;
				_cbstate.cbroot = (heap_cacheblock_t*)_cbstate.cbtop;
				return _cbstate.cbtop;
		} else {
			newblk = __heap_find_cacheblock();
			if (newblk) {
				newblk->busy_mru = true;
				return newblk;
			}
			if ((uint32_t)_cbstate.cbtop + sizeof(heap_cacheblock_t) > _cbstate.end) {
				return NULL;
			}
			newblk = (heap_cacheblock_t*)_cbstate.cbtop + sizeof(heap_cacheblock_t);
			newblk->prev = _cbstate.cbtop;
			_cbstate.cbtop->next = newblk;
			_cbstate.cbtop = newblk;
			newblk->busy_mru = true;
			return newblk;
		}

}


static void __heap_free_cacheblock(heap_cacheblock_t* cb) {
	cb->busy_mru = false;
}

heap_resblock_t* __heap_find_resblock() {
	heap_resblock_t *cur;
	cur = _rbstate.rbroot;
	while(cur != NULL && cur->busy_mru) {
		cur = (heap_resblock_t*)cur + sizeof(heap_resblock_t);
	}
	return cur;

}

heap_resblock_t* __heap_alloc_resblock() {
	heap_resblock_t *newblk;
	if (_rbstate.rbroot == NULL) {
		printk("rbroot = null\n");
		_rbstate.rbtop = (heap_resblock_t*)_rbstate.begin;
		_rbstate.rbroot = (heap_resblock_t*)_rbstate.rbtop;
		printk("rbroot = 0x%x\n",_rbstate.rbroot);
		return _rbstate.rbtop;
	} else {
		printk("rbroot != null\n");
		newblk = __heap_find_resblock();
		if (newblk) {
			printk("find newblk\n");
			newblk->busy_mru = true;
			return newblk;
		}
		if ((uint32_t)_rbstate.rbtop + sizeof(heap_resblock_t) > _rbstate.end) {
			printk("> end\n");
			return NULL;
		}
		newblk = (heap_resblock_t*)_rbstate.rbtop + sizeof(heap_resblock_t);
//			newblk->prev = _rbstate.rbtop;
//			_rbstate.rbtop->next = newblk;
		_rbstate.rbtop = newblk;
		newblk->busy_mru = true;
		return newblk;
	}

}

void __heap_free_resblock(heap_resblock_t* rb) {
	rb->busy_mru = false;
}


void __gheap_init() {
	_rbstate.begin = PAGEINFO(heap_blks_pgt1[0],frame);
	//printk("0x%x",PAGEINFO(heap_blks_pgt1[0],frame));
	_rbstate.end = PAGEINFO(heap_blks_pgt1[11],frame) - 1;

	_cbstate.begin = PAGEINFO(heap_blks_pgt1[11],frame); // 10(11) - 2000 resources blocks
		_cbstate.end = PAGEINFO(heap_blks_pgt1[1023],frame) + 4096 - 1; // other - cache blocks
	__heap_autorescue_mode = true;
}


#define lbtomem(lb) ((void*)lb + sizeof(heap_localblock_t))
#define memtolb(mem) ((heap_localblock_t*)mem - sizeof(heap_localblock_t))

void __heap_dump_cacheblock(heap_cacheblock_t* cb) {
	printk("{ .size = %d, .busy = %d, .busy_mru = %d, .lb = 0x%x, .res = 0x%x, .next = 0x%x, .prev = 0x%x }",
	       	cb->size,cb->busy,cb->busy_mru,cb->localblock, cb->resource, cb->next, cb->prev);

}
void __heap_dump_resblock(heap_resblock_t* rb) {
	printk("{ .pages = %d, .rcb = 0x%x, .tcb = 0x%x, .busy_mru = 0x%x, .resid = 0x%x, .beginpf = 0x%x, .lastpf = 0x%x }",
	       	rb->pages, rb->root_cacheblock, rb->top_cacheblock, rb->busy_mru, rb->resid, rb->beginpf, rb->lastpf);
}


void* __heap_alloc(heap_resblock_t* resb, size_t size) {
	printk("allocator started\n");
	if(resb == NULL) { panice("empty resource block"); return NULL; }
	__heap_dump_resblock(resb);
	heap_cacheblock_t** curp = &resb->root_cacheblock;
	heap_cacheblock_t** prevp = &resb->root_cacheblock;
	heap_cacheblock_t* cur = *curp;

	while(*curp != NULL) {
		printk("<0x%x:0x%x>\n",curp,*curp);
		__heap_dump_cacheblock(*curp);
		printk("\n\n");
		if(cur->localblock->cacheblock != cur) {
			if(__heap_autorescue_mode) {
				cur->localblock->cacheblock = cur;
			} else {
				panice("dead heap block");
			}
		}
		if(cur->busy) goto nextel;

		if(cur->size == size) {
			cur->busy = true;
			return lbtomem(cur->localblock);
		}
		if(cur->size > size) {
			heap_cacheblock_t *blk = __heap_alloc_cacheblock();
			if (blk == NULL) { cur->busy = true; return cur; }
			blk->size = cur->size - size;
			cur->size = size;
			blk->busy = false;
			blk->resource = resb;
			blk->prev = cur;
			blk->next = cur->next;
			cur->next = blk;
			blk->localblock = (heap_localblock_t*)((char*)lbtomem(((heap_cacheblock_t*)*prevp)) + size);
			cur->busy = true;
			return lbtomem(cur->localblock);
		}

	nextel:;
		printk("prevp:0x%x\n",cur);
		prevp = curp;
		printk("prevpn:0x%x~0x%x\n",prevp,*prevp);
		printk("curp:0x%x~0x%x\n",curp,*curp);
		curp = &cur->next;
		printk("curpn:0x%x~0x%x\n",curp,*curp);
		printk("cur:0x%x\n",cur);
		cur = cur->next;
		printk("curn:0x%x\n",cur);
	}

	if(*prevp != NULL && ((heap_cacheblock_t*)*prevp)->localblock + resb->top_cacheblock->size > resb->lastpf + 4095) {
		return NULL;
	}

	heap_cacheblock_t *blk = __heap_alloc_cacheblock();
	if(blk == NULL) { return NULL; }
	// blk->prev = prevp == curp ? NULL : *prevp;
	// blk->next = NULL;

	// blk->resource = resb;
	// blk->size = size;
	// blk->localblock = prevp == curp ? (heap_localblock_t*)resb->beginpf : (lbtomem(((heap_cacheblock_t*)*prevp)->localblock) + size > resb->lastpf + 4095 ? NULL : lbtomem(((heap_cacheblock_t*)*prevp)->localblock) + size );
	// if(!blk->localblock) {
	// 	__heap_free_cacheblock(blk);
	// 	return NULL;
	// }
	// if(resb->root_cacheblock == NULL) { resb->root_cacheblock = blk;}
	// resb->top_cacheblock = blk;
	blk->busy = true;
	blk->busy_mru = true;
	blk->size = size;
	blk->localblock = prevp == curp ? \
		(heap_localblock_t*)resb->beginpf : \
		(heap_localblock_t*)lbtomem(resb->top_cacheblock->localblock) + resb->top_cacheblock->size;
	blk->resource = resb;

	blk->next = NULL;
	blk->prev = prevp == curp ? NULL : *prevp;
	if(prevp != curp) {
		((heap_cacheblock_t*)*prevp)->next = blk;
	}

	if(!resb->root_cacheblock) {
		resb->root_cacheblock = blk;
	}
	resb->top_cacheblock = blk;

	printk("<<0x%x>>",lbtomem(blk->localblock));

	return lbtomem(blk->localblock);
}
void __heap_free(void* ptr) {
	heap_localblock_t *lb = memtolb(ptr);
	heap_cacheblock_t *cb = lb->cacheblock;
	if (cb->localblock != lb) {
		if (__heap_autorescue_mode) {
			return;
		} else {
			panice("dead heap block");
		}
	}
	cb->busy = false;
}

