#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mymalloc.h"


double rnd_flt(){
    return (double) rand() / RAND_MAX;
}


/**
 * chooses between zero to max
 * @param max
 * @return choosen number
 */
int rnd_choice(int max){
    return 0 + rand() / (RAND_MAX / (max - 0 + 1) + 1);
}


int rnd_size(){
    return rnd_choice(254);
}

// last three choices
void* l1 = NULL;
void* l2 = NULL;
void* l3 = NULL;


void set_pre_on_turn(int turn,void* ptr){
    int turn_m = turn%3;
    if(turn_m == 0)
        l1 = ptr;
    else if(turn_m == 1)
        l2 = ptr;
    else
        l3 = ptr;
}

void* get_rnd_ptr(){
    int r = rnd_choice(2);
    if(r == 0)
        return l1;
    else if(r == 1)
        return l2;
    else
        return l3;
}


void do_test(){
    l1 = NULL;
    l2 = NULL;
    l3 = NULL;
    for (int i = 0; i < 1000; ++i) {

        int op = rnd_choice(2); //0 alloc //1 realloc //2free
        int size = rnd_size();

        if(op == 0)
            set_pre_on_turn(i,mymalloc(size));
        else if(op == 1)
            set_pre_on_turn(i, myrealloc(get_rnd_ptr(),size));
        else
            myfree(get_rnd_ptr());

//        printf("%i\n",i);

    }
}

void perf_first_fit(){
    myinit(FIRST_FIT);

    clock_t start =  clock();
    do_test();
    clock_t end = clock();

    double total_sec = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("First fit throughput: %f ops/sec\n" , (double) 1000/total_sec);
    printf("First fit utilization: %f\n" , utilization());

    mycleanup();
}

void perf_next_fit(){
    myinit(NEXT_FIT);
    clock_t start =  clock();
    do_test();
    clock_t end = clock();

    double total_sec = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("Next fit throughput: %f ops/sec\n" , (double) 1000/total_sec);
    printf("Next fit utilization: %f\n" , utilization());

    mycleanup();
}
void perf_best_fit(){
    myinit(BEST_FIT);
    clock_t start =  clock();
    do_test();
    clock_t end = clock();

    double total_sec = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("Best fit throughput: %f ops/sec\n" , (double) 1000/total_sec);
    printf("Best fit utilization: %f\n" , utilization());

    mycleanup();
}
int main(){
    set_silent_on_wrong_free(0);

    perf_first_fit();
    perf_next_fit();
    perf_best_fit();
}