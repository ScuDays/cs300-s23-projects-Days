#include "dmalloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
// heap_min and heap_max checking, no overlap with other regions.

static int global;

int main() {
    for (int i = 0; i != 100; ++i) {
        size_t sz = rand() % 100;
        char* p = (char*) malloc(sz);
        free(p);

         dmalloc_stats stat;
    get_statistics(&stat);
    printf("%10llu %10llu %10llu\n",sz,stat.heap_min, stat.heap_max);
    }
   


}
