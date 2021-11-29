//
// Created by human on 11/27/21.
//

#ifndef UNTITLED_MYMALLOC_H
#define UNTITLED_MYMALLOC_H

#include <stddef.h>

#define HEAP_SIZE 1024*1024



enum {
    FIRST_FIT,
    NEXT_FIT,
    BEST_FIT
};

void myinit(int allocAlg);
void* mymalloc(size_t size);
void myfree(void* ptr);
void* myrealloc(void* ptr,size_t size);
void mycleanup();
double utilization();


void set_silent_on_wrong_free(int);
void set_exit_on_wrong_free(int);

#endif //UNTITLED_MYMALLOC_H
