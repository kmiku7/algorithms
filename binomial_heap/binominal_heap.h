#ifndef __BINOMINAL_HEAP_H__
#define __BINOMINAL_HEAP_H__
#include <stdlib.h>

typedef int BH_KEY;
typedef void*   BH_VALUE;
typedef unsigned int    BH_RANK_TYPE;
struct __binominal_heap_node{
    BH_RANK_TYPE    rank;
    BH_KEY  key;
    BH_VALUE    value;
    struct __binominal_heap_node   *child;
    struct __binominal_heap_node   *sibling;
    struct __binominal_heap_node   *parent;
};

typedef struct __binominal_heap_node*   BH_NODE;
typedef BH_NODE BH_HEAD;
typedef void (*BH_RELEASE_VALUE)(BH_VALUE);

BH_HEAD create_binominal_heap();
void destroy_heap(BH_HEAD* p_heap, BH_RELEASE_VALUE release);
int bh_empty(BH_HEAD heap);
BH_KEY bh_find_min_key(BH_HEAD heap);
BH_VALUE bh_find_min_value(BH_HEAD heap);
BH_VALUE bh_delete_min(BH_HEAD heap);
BH_NODE bh_insert(BH_HEAD heap, BH_KEY key, BH_VALUE value);
BH_HEAD bh_merge(BH_HEAD left, BH_HEAD right);
void bh_decrease_key(BH_HEAD heap, BH_NODE node, BH_KEY key);

void print_tree(BH_HEAD heap);

#endif // __BINOMINAL_HEAP_H__
