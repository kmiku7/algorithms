#include <stdio.h>
#include "binominal_heap.h"

int main(int argc, char** argv) {

    printf("----- TEST -----\n");

    BH_HEAD heap = create_binominal_heap();
    printf("is empty: %d\n", bh_empty(heap));
    bh_insert(heap, 10, NULL);
    bh_insert(heap, 4, NULL);
    bh_insert(heap, 14, NULL);
    bh_insert(heap, 422, NULL);
    bh_insert(heap, 42, NULL);
    bh_insert(heap, 46, NULL);
    BH_NODE node_4222 = bh_insert(heap, 4222, NULL);
    bh_insert(heap, 433, NULL);
    bh_preorder_print_heap(heap, stdout);
    printf("min: %d\n", bh_find_min_key(heap));
    bh_delete_min(heap);
    bh_preorder_print_heap(heap, stdout);
    printf("min: %d\n", bh_find_min_key(heap));
    bh_delete_min(heap);
    bh_preorder_print_heap(heap, stdout);
    printf("min: %d\n", bh_find_min_key(heap));

    bh_decrease_key(heap, node_4222, 0);
    bh_preorder_print_heap(heap, stdout);
    printf("min: %d\n", bh_find_min_key(heap));

    destroy_heap(&heap, NULL);
    return 0;
}
