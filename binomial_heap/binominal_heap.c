#include "binominal_heap.h"
#include <assert.h>

// private
void __bh_release_node(BH_NODE node, BH_RELEASE_VALUE release) {
    if(node && release) {
        (*release)(node->value);
    }
    free(node);
}
BH_NODE __bh_new_node() {
    return (BH_NODE)calloc(1, sizeof(struct __binominal_heap_node));
}

BH_NODE __bh_find_min_node(BH_HEAD heap){
    assert(heap!=NULL && heap->sibling!=NULL);

    BH_NODE min_node = heap->sibling;
    BH_NODE curr_node = min_node->sibling;
    while(curr_node) {
        if(curr_node->key < min_node->key)
            min_node = curr_node;
        curr_node = curr_node->sibling;
    }
    return min_node;
}

BH_NODE __bh_node_merge(BH_HEAD left, BH_HEAD right) {
    BH_NODE left_list = left->sibling;
    left->sibling = NULL;
    BH_NODE right_list = right->sibling;
    right->sibling = NULL;

    BH_NODE prev_node = left;
    while(left_list && right_list) {
        if(left_list->rank < right_list->rank) {
            prev_node->sibling = left_list;
            prev_node = left_list;
            left_list = left_list->sibling;
        } else {
            prev_node->sibling = right_list;
            prev_node = right_list;
            right_list = right_list->sibling;
        }
    }
    prev_node->sibling = left_list ? left_list : right_list;
    BH_NODE list_head = left->sibling;
    left->sibling = NULL;
    return list_head;
}

//public
BH_HEAD create_binominal_heap() {
    BH_NODE node = __bh_new_node();
    return node;
}
void destroy_heap(BH_HEAD* p_heap, BH_RELEASE_VALUE release) {
    if(p_heap==NULL || *p_heap==NULL) return;

    BH_HEAD heap = *p_heap;
    BH_NODE node = heap->sibling;

    while(node) {
        if(node->child!=NULL) {
            BH_NODE next = node->child;
            node->child = NULL;
            node = next;
        } else {
            BH_NODE next = node->sibling;
            if(node->sibling==NULL)
                next = node->parent;
            __bh_release_node(node, release);
            node = next;
        }
    }

    __bh_release_node(heap, NULL);
    p_heap = NULL;
}

int bh_empty(BH_HEAD heap) {
    assert(heap!=NULL);
    return heap->sibling==NULL;
}

BH_KEY bh_find_min_key(BH_HEAD heap) {
    return __bh_find_min_node(heap)->key;
}

BH_VALUE bh_find_min_value(BH_HEAD heap) {
    return __bh_find_min_node(heap)->value;
}

BH_VALUE bh_delete_min(BH_HEAD heap) {
    assert(heap!=NULL && heap->sibling!=NULL);
    BH_NODE min_node_prev_sib = heap;
    BH_NODE min_node = heap->sibling;
    BH_NODE p_next_node = min_node;
    while(p_next_node->sibling) {
        if(p_next_node->sibling->key < min_node->key){
            min_node_prev_sib = p_next_node;
            min_node = p_next_node->sibling;
        }
        p_next_node = p_next_node->sibling;
    }

    // binominal tree的兄弟节点按照rank递减排序，根链按照递增排序。
    // 因此tree->heap的时候需要reverse(node->sibling)
    BH_NODE sub_heap = min_node;
    min_node_prev_sib->sibling = min_node->sibling;
    sub_heap->sibling = sub_heap->child;
    sub_heap->child = NULL;
    BH_NODE curr_node = sub_heap->sibling;
    sub_heap->sibling = NULL;
    while(curr_node){
        curr_node->parent = NULL;
        BH_NODE next_node = curr_node->sibling;
        curr_node->sibling = sub_heap->sibling;
        sub_heap->sibling = curr_node;
        curr_node = next_node;
    }

    BH_HEAD tmp_heap = bh_merge(heap, sub_heap);
    *heap = *tmp_heap;
    __bh_release_node(tmp_heap, NULL);

    BH_VALUE tmp_value = sub_heap->value;
    __bh_release_node(sub_heap, NULL);
    return tmp_value;
}

BH_NODE bh_insert(BH_HEAD heap, BH_KEY key, BH_VALUE value) {
    BH_HEAD tmp_heap = create_binominal_heap();
    BH_NODE new_node = __bh_new_node();
    new_node->key = key;
    new_node->value = value;
    new_node->rank = 0;
    tmp_heap->sibling = new_node;
    BH_HEAD merged_heap = bh_merge(heap, tmp_heap);
    *heap = *merged_heap;
    __bh_release_node(tmp_heap, NULL);
    __bh_release_node(merged_heap, NULL);
    return new_node;
}

BH_HEAD bh_merge(BH_HEAD left, BH_HEAD right) {
    assert(left!=NULL && right!=NULL);

    BH_HEAD new_heap = create_binominal_heap();
    new_heap->sibling = __bh_node_merge(left, right);

    if(new_heap->sibling==NULL)
        return new_heap;

    BH_NODE prev_node = new_heap;
    BH_NODE curr_node = new_heap->sibling;
    while(curr_node->sibling) {
        BH_NODE next_node = curr_node->sibling;
        if((curr_node->rank != next_node->rank)
            || (next_node->sibling!=NULL && next_node->sibling->rank==next_node->rank)){
            prev_node = curr_node;
            curr_node = next_node;
        } else if(curr_node->key < next_node->key) {
            curr_node->sibling = next_node->sibling;
            next_node->sibling = curr_node->child;
            next_node->parent = curr_node;
            curr_node->child = next_node;
            curr_node->rank += 1;
        } else {
            // curr_node->key > next_node->key
            curr_node->sibling = next_node->child;
            curr_node->parent = next_node;
            next_node->child = curr_node;
            prev_node->sibling = next_node;
            next_node->rank += 1;
            curr_node = next_node;
        }
    }
    return new_heap;
}

void bh_decrease_key(BH_HEAD heap, BH_NODE node, BH_KEY key) {
    assert(heap!=NULL && node!=NULL && key < node->key);
    // shift up
    BH_VALUE value = node->value;
    while(node->parent && key < node->parent->key) {
        node->key = node->parent->key;
        node->value = node->parent->value;
        node = node->parent;
    }
    node->key = key;
    node->value = value;
}

void __bh_print_node(BH_NODE node, FILE* fout) {
    fprintf(fout, "(id:%p, r:%u, k:%d, v:%p, p:%p, c:%p, s:%p)\n", 
        node, node->rank, node->key, node->value, node->parent, node->child, node->sibling);
}

void bh_preorder_print_heap(BH_HEAD heap, FILE* fout) {
    assert(fout != NULL);
    if(heap == NULL) {
        fprintf(fout, "INVALID\n");
        return;
    }

    fprintf(fout, "HEAP(%p)\n", heap);

    BH_NODE curr = heap->sibling;
    while(curr != NULL) {
        __bh_print_node(curr, fout);
        BH_NODE child = curr->child;
        if(child != NULL) {
            while(child!=NULL) {
                //assert(child->parent == curr);
                child = child->sibling;
            }
            curr = curr->child;
        } else if(curr->sibling != NULL) {
            curr = curr->sibling;
        } else if(curr->parent) {
            curr = curr->parent->sibling;
        } else {
            break;
        }
    }
    fprintf(fout, "-----\n");
}
