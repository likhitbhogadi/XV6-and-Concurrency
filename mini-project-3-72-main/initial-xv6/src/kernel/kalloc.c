// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct spinlock ref_lock;
int ref_count[PHYSTOP/PGSIZE];
struct spinlock cow_pagefault_count_lock;
int cow_pagefault_count;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&cow_pagefault_count_lock, "cow_pagefault_count_lock");
  initlock(&ref_lock, "ref_lock"); // initializing the lock for ref_count
  freerange(end, (void*)PHYSTOP);
  // int cow_pagefault_count = 0;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){

    acquire(&ref_lock);
    ref_count[((uint64)p) / PGSIZE] = 1;
    release(&ref_lock);
    kfree(p);
  }
}


// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // decrement ref_count
  acquire(&ref_lock);
  ref_count[(uint64)pa/PGSIZE]--;
  int count = ref_count[(uint64)pa/PGSIZE];

  // if ref_count is 0 after decrement, free , else don't
  if(count != 0 ){
    release(&ref_lock);
    return;
  }
  release(&ref_lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){

    memset((char*)r, 5, PGSIZE); // fill with junk
    // initialize ref_count to 0
    acquire(&ref_lock);
    ref_count[(uint64)r/PGSIZE] = 1;
    release(&ref_lock);
  }
  return (void*)r;
}
