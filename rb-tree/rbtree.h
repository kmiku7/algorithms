#ifndef __RB_TREE_HEADER__
#define __RB_TREE_HEADER__

#include <cstddef>

const int ROOT = 0;
const int RED = 1;
const int BLACK = 2;


struct RBTreeNode {
    size_t  key;
    size_t  value;
    int     color;
    RBTreeNode *left, *right;
};


RBTreeNode* rb_create_tree();

void rb_destroy_tree(RBTreeNode* root);

bool rb_insert_node(RBTreeNode* root, size_t key, size_t value);

bool rb_delete_node(RBTreeNode* root, size_t key, size_t* pvalue);

void rb_print_tree(RBTreeNode* root);

void rb_check_tree(RBTreeNode* node, bool init = true, 
    int len = 0, size_t range_begin = 0, size_t range_end = ~(0llu));

#endif // __RB_TREE_HEADER__
