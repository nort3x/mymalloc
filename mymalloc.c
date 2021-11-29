#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "mymalloc.h"


typedef int header;


inline int is_used(header *h);

inline void set_used(header *h);

inline void clear_used(header *h);

inline void write_size(header *h, int size);

inline int read_size(header *h);

inline void *alloc(header *h, int size);

inline void dealloc(header *h);

inline header *next_header(header *h);

int silent_on_wrong_free = 0;
int exit_on_wrong_free = 0;

void set_silent_on_wrong_free(int x){
    silent_on_wrong_free = x;
}
void set_exit_on_wrong_free(int x){
    exit_on_wrong_free = x;
}

typedef struct {
    int alg;
    char heap[HEAP_SIZE];

    header *first_block;
    header *last_block;

    header *last_allocation_ptr;

} data;

data *d;

void myinit(int _alg) {

    d = malloc(sizeof(data));
    d->alg = _alg;
    d->first_block = (header *) d->heap;
    d->last_allocation_ptr = (header *) d->heap;
    d->last_block = (header *) ((d->heap) + HEAP_SIZE);


    write_size(d->first_block, HEAP_SIZE - ((int) sizeof(header)));
    clear_used(d->first_block);


//    printf("heap %p\n", d->heap);
//    printf("firstblock %p\n", d->first_block);
//    printf("lastblock %p\n", d->last_block);
}

inline header *get_smallest_allocation_able_block(int size);

int is_alloc_able(header *h, int size) {
    if (is_used(h) || read_size(h) < size) {
        return 0;
    } else
        return 1;
}


void *mymalloc(size_t size) {

    if (size == 0) return NULL;

    if (d->alg == FIRST_FIT) {
        header *ptr = d->first_block;
        while (ptr != d->last_block) {
            if (is_alloc_able(ptr, size))
                return alloc(ptr, size);
            ptr = next_header(ptr);
        }
        if (is_used(ptr)) return NULL;


    } else if (d->alg == NEXT_FIT) {
        //basically it's exactly like FIRST_FIT (BUT) rotates around last_allocation_ptr
        // beware next_header method will return first_block if giving it last_block
        // meaning it will rerun the recursion
        // by substation of last_block as last_allocation_ptr we end up with above branch
        header *ptr = d->last_allocation_ptr;
        do {
            if (is_alloc_able(ptr, size))
                return alloc(ptr, size);
            ptr = next_header(ptr);
        } while (ptr != d->last_allocation_ptr);

        if (is_used(ptr)) return NULL;
    } else {
        header *h = get_smallest_allocation_able_block(size);
        if (h == NULL)return NULL;
        return alloc(h, size);
    }

    return NULL; // in case no strategy is defined
}

void myfree(void *ptr) {
    if(ptr==NULL) return;

    header *associated_header = (header *) (ptr - sizeof(header));

    if (ptr < (void *) d->first_block || ptr > (void *) d->last_block) {
        if(silent_on_wrong_free)
            printf("i don't free what does NOT belong to me\n");
        if(exit_on_wrong_free)
            exit(-1);
        return;
    }


    if (!is_used(associated_header)) {
        if(silent_on_wrong_free)
            printf("not being used, double free or blind bruteforce ?\n");

        if(exit_on_wrong_free)
            exit(-1);
        return;
    }


    dealloc(associated_header);
}

void *myrealloc(void *ptr, size_t size) {
    if(ptr==NULL) return NULL;


    header *h = (header *) (ptr - sizeof(header));

    int total_over_size = ((int) size) - read_size(h);
    // return if this buffer is actually spacier than requester needs
    // mod 8 constraint allocate the same buffer for 4,5,6,7 requests, it's a wise thing to do
    if (total_over_size <= 0) return ptr;


    header *next_h = next_header(h);

    int next_buffer_flow = abs(total_over_size - (int) sizeof(header));
    if (is_alloc_able(next_h, next_buffer_flow)) {
        // in this branch we have next buffer free and available for allocation
        // we just call alloc on our current buffer and give new size
        // alloc will resize our current header and make next header if possible
        return alloc(h, size);
    }
    // if all above strategies fail we have to reallocate an actual buffer
    // copy all containment of this buffer
    // and free this buffer

    void *ans = mymalloc(size);
    if (ans == NULL) return ans; // when can't make a new buffer, we keep current data as is and reject callers request

    // copy data to new location

    int last_buffer_size = read_size(h);

    char *ans_as_char = (char *) ans;
    char *ptr_as_char = (char *) ptr;

    for (int i = 0; i < last_buffer_size; ++i) {
        ans_as_char[i] = ptr_as_char[i];
    }


    // free current ptr

    myfree(ptr);

    // brand-new buffer to play with
    return ans;

}


void mycleanup() {
    free(d);
}

double utilization() {

    int used_mem = 0;
    int free_mem = 0;
    header *ptr = d->first_block;
    while (1) {
        header *_next_header = next_header(ptr);
        if (_next_header == d->last_block)
            break;

        if (is_used(ptr))
            used_mem += read_size(ptr);
        else
            free_mem += read_size(ptr);


        ptr = _next_header;
    }


    int total_heap_usage = (int)(((void*)ptr - (void*)d->first_block) + sizeof(header));
    return ((double) used_mem) / (total_heap_usage - free_mem) ;

}

void *alloc(header *h, int size) {
    if (size % 4 != 0)
        size = (size / 4 + 1) * 4;

    int current_size = read_size(h);

    // allocate this header
    set_used(h);
    write_size(h, (int) size);

    // if possible make other header
    if (current_size - size - ((int) sizeof(header)) > 0) {

        header *nxt_ptr = (header *) ((void *) h + sizeof(header) + size);
        int nxt_size = current_size - size - ((int) sizeof(header));
        write_size(nxt_ptr, nxt_size);
        clear_used(nxt_ptr);


    }

    d->last_allocation_ptr = h;
    return ((void *) h) + sizeof(header);
}

void dealloc(header *h) {
    header *right = next_header(h);

    // if next block is also free, merge!
    if (!is_used(right)) {
        write_size(h, read_size(h) + read_size(right) + sizeof(header));
    }

    // we should consider a situation which non-of left of right are available, we just free this block
    clear_used(h);

}

header *next_header(header *h) {
    if (h == d->last_block) return d->first_block;
    return ((void *) h) + sizeof(header) + read_size(h);
}


void clear_used(header *h) {
    *h = is_used(h) ? -*h : *h;
}

int is_used(header *h) {
    if (h == d->last_block) return 1;
    return *h > 0 ? 1 : 0;
}

void set_used(header *h) {
    *h = is_used(h) ? *h : -*h;
}

void write_size(header *h, int size) {
    *h = size;
}

int read_size(header *h) {
    if (h == d->last_block) return 0;
    return abs(*h);
}


/**
 * will search entire heap for smallest block which can be allocated for given size
 * or Null if no block can be found
 * @param size
 * @return smallest allocatable block in heap
 */
header *get_smallest_allocation_able_block(int size) {

    header *starting_point = d->first_block;
    header *stop_at = d->last_block;

    header *floating_ptr = starting_point;
    header *answer = NULL;

    int float_size = -1;
    while (1) {
        if (is_alloc_able(floating_ptr, size))
            if (float_size == -1 || float_size > read_size(floating_ptr)) {
                float_size = read_size(floating_ptr);
                answer = floating_ptr;
            }
        floating_ptr = next_header(floating_ptr);
        if (floating_ptr == stop_at) break;
    }

    return answer == NULL || is_used(answer) ? NULL : answer;

}