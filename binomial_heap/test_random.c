#include <stdio.h>
#include "binominal_heap.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int int_compare(const void* left, const void* right) {
    int left_value = *(const int*)left;
    int right_value = *(const int*)right;
    if(left_value < right_value)
        return -1;
    else if(left_value == right_value)
        return 0;
    return 1;
}

void should_not_be_called(BH_VALUE value){
    assert(0);
}

int main(int argc, char** argv) {

    int random_size = 1000000;
    int *buffer = malloc(sizeof(int)*random_size);

    srandom(time(NULL));
    BH_HEAD heap_left = create_binominal_heap();
    BH_HEAD heap_right = create_binominal_heap();
    for(int i=0; i<random_size; ++i) {
        int key = (int)random();
        buffer[i] = key;
        if(i&0x1){
            bh_insert(heap_left, key, (BH_VALUE)key);
        } else {
            bh_insert(heap_right, key, (BH_VALUE)key);
        }
    }

    qsort(buffer, random_size, sizeof(int), int_compare);
    //bh_preorder_print_heap(heap_left, stdout);
    //bh_preorder_print_heap(heap_right, stdout);
    BH_HEAD heap = bh_merge(heap_left, heap_right);
    //bh_preorder_print_heap(heap, stdout);
    destroy_heap(&heap_left, should_not_be_called);
    destroy_heap(&heap_right, should_not_be_called);

    for(int i=0; i<random_size; ++i) {
        assert(bh_empty(heap)==0);
        int key = (int)bh_find_min_key(heap);
        int value = (int)bh_find_min_value(heap);
        assert(key==buffer[i] && (int)value == buffer[i]);
        bh_delete_min(heap);
    }

    destroy_heap(&heap, should_not_be_called);
    return 0;
}
