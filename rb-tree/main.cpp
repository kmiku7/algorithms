#include "rbtree.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <climits>

using namespace std;



int main(int argc, char** argv) {

    const int size = 1500;
    size_t items[size] = {15, 2, 12, 14, 6, 10, 8, 9, 5, 7, 11, 13, 3, 4, 1};

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    for(int i=0; i<size; ++i) {
        items[i] = i+1;
    }

    auto rander = std::default_random_engine(seed);
    shuffle(items, items+size, rander);

    RBTreeNode* root = rb_create_tree();

    for(int i=0; i<size; ++i) {
        printf("%lu, ", items[i]);
    }
    printf("\n");

    printf("start insert\n");
    for(int i=0; i<size; ++i) {
        rb_insert_node(root, items[i], items[i]);
        rb_print_tree(root);
        rb_check_tree(root);
    }

    printf("start delete\n");
    for(int i=0; i<size; ++i) {
        size_t value;
        printf("DELTE: %lu\n", items[i]);
        rb_delete_node(root, items[i], &value);
        rb_print_tree(root);
        rb_check_tree(root);
    }

    printf("start insert\n");
    for(int i=0; i<size; ++i) {
        rb_insert_node(root, items[i], items[i]);
        rb_print_tree(root);
        rb_check_tree(root);
    }

    rb_destroy_tree(root);
    root = NULL;

    return 0;
    

}

/*

int main(int argc, char** argv) {

    RBTreeNode* root = rb_create_tree();

    rb_insert_node(root, 12, 12);
    rb_check_tree(root);
    rb_insert_node(root, 3, 3);
    rb_check_tree(root);
    rb_insert_node(root, 4, 4);
    rb_check_tree(root);

    rb_print_tree(root);

    return 0;

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
