#include "rbtree.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;



int main(int argc, char** argv) {

    const int size = 15;
    size_t items[size] = {15, 2, 12, 14, 6, 10, 8, 9, 5, 7, 11, 13, 3, 4, 1};

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    for(int i=0; i<size; ++i) {
        //items[i] = i+1;
    }

    //shuffle(items, items+size, std::default_random_engine(seed));

    RBTreeNode* root = rb_create_tree();

    for(int i=0; i<size; ++i) {
        rb_insert_node(root, items[i], items[i]);
        rb_check_tree(root);
    }

    for(int i=0; i<size; ++i) {
        size_t value;
        printf("DELTE: %lu\n", items[i]);

        rb_delete_node(root, items[i], &value);
        rb_print_tree(root);
        rb_check_tree(root);
    }

    return 0;
    

}

/*
int main(int argc, char** argv) {

    RBTreeNode* root = rb_create_tree();

    rb_insert_node(root, 1, 1);
    rb_check_tree(root);
    rb_insert_node(root, 2, 2);
    rb_check_tree(root);
    rb_insert_node(root, 3, 3);
    rb_check_tree(root);
    rb_insert_node(root, 4, 4);
    rb_check_tree(root);

    rb_print_tree(root);

    size_t value;
    rb_delete_node(root, 1, &value);
    rb_check_tree(root);
    rb_print_tree(root);
    rb_delete_node(root, 2, &value);
    rb_check_tree(root);
    rb_print_tree(root);
    rb_delete_node(root, 3, &value);
    rb_check_tree(root);
    rb_print_tree(root);
    rb_delete_node(root, 4, &value);
    rb_check_tree(root);
    rb_print_tree(root);

    return 0;
}
*/
