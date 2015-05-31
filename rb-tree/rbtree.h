#ifndef __RB_TREE_HEADER__
#define __RB_TREE_HEADER__

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <climits>

using std::printf;
using std::calloc;
using std::free;

// 没有重复元素。

const int RB_TREE_MAX_HEIGHT = 128;
const int ROOT = 0;
const int RED = 1;
const int BLACK = 2;


struct RBTreeNode {
    size_t  key;
    size_t  value;
    int     color;
    RBTreeNode *left, *right;
};


// node->right指向根元素。
RBTreeNode* rb_create_tree() {
    RBTreeNode* node = static_cast<RBTreeNode*>(calloc(1, sizeof(RBTreeNode)));
    node->color = ROOT;
    return node;
}


void rb_destroy_tree(RBTreeNode* root) {
    assert(root!=NULL);
    RBTreeNode* node = root;
    root = NULL;
    while(node) {
        if(node->left==NULL) {
            RBTreeNode* temp = node->right;
            free(node);
            node = temp;
        } else {
            RBTreeNode* precessor = node->left;
            while(precessor->right != NULL && precessor->right != node) {
                precessor = precessor->right;
            }
            if(precessor->right == NULL) {
                precessor->right = node;
                node = node->left;
            } else {
                RBTreeNode* temp = node->right;
                free(node);
                node = temp;
            }
        }
    }
}

bool rb_insert_node(RBTreeNode* root, size_t key, size_t value) {
    RBTreeNode* traverse_path[RB_TREE_MAX_HEIGHT];
    int traverse_path_len = 0;
    RBTreeNode* curr = root->right;
    while(curr) {
        traverse_path[traverse_path_len++] = curr;
        assert(curr->key != key);
        if(key < curr->key)
            curr = curr->left;
        else 
            curr = curr->right;
    }

    RBTreeNode* new_node = static_cast<RBTreeNode*>(calloc(1, sizeof(RBTreeNode)));
    new_node->key = key;
    new_node->value = value;
    new_node->color = RED;

    if(traverse_path_len==0) {
        root->right = new_node;
        new_node->color = BLACK;
    } else {
        RBTreeNode* parent = traverse_path[traverse_path_len-1];
        if(key < parent->key) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }

        // fix
        

        while(traverse_path_len > 1) {
            parent = traverse_path[traverse_path_len-1];
            if(parent->color == BLACK)
                return true;

            RBTreeNode* pparent = traverse_path[traverse_path_len-2];
            if(parent == pparent->left) {
                RBTreeNode* uncle = pparent->right;
                if(uncle && uncle->color == RED) {
                    uncle->color = BLACK;
                    parent->color = BLACK;
                    pparent->color = RED;

                    new_node = pparent;
                    traverse_path_len -= 2;
                } else {
                    // uncle->color == BLACk;
                    if(new_node == parent->right) {
                        // rotate left;
                        // 有“父指针”的旋转。
                        parent->right = new_node->left;
                        new_node->left = parent;
                        pparent->left = new_node;
                        
                        parent = pparent->left;
                        new_node = parent->left;
                    }

                    // rotate right;
                    // 无“父指针”的旋转。
                    size_t pparent_key = pparent->key;
                    size_t pparent_value = pparent->value;
                    pparent->key = parent->key;
                    pparent->value = parent->value;
                    pparent->left = parent->left;
                    parent->left = parent->right;
                    parent->right = pparent->right;
                    pparent->right = parent;
                    parent->key = pparent_key;
                    parent->value = pparent_value;
                    return true;
                }
            } else if(parent == pparent->right) {
                RBTreeNode* uncle = pparent->left;
                if(uncle && uncle->color == RED) {
                    uncle->color = BLACK;
                    parent->color = BLACK;
                    pparent->color = RED;
                    
                    new_node = pparent;
                    traverse_path_len -= 2;
                } else {
                    // uncle->color == BLACK;
                    if(new_node == parent->left) {
                        parent->left = new_node->right;
                        new_node->right = parent;
                        pparent->right = new_node;

                        parent = pparent->right;
                        new_node = parent->right;
                    }

                    size_t pparent_key = pparent->key;
                    size_t pparent_value = pparent->value;
                    pparent->key = parent->key;
                    pparent->value = parent->value;
                    pparent->right = parent->right;
                    parent->right = parent->left;
                    parent->left = pparent->left;
                    pparent->left = parent;
                    parent->key = pparent_key;
                    parent->value = pparent_value;
                    return true;
                }
            } else {
                assert(false);
            }
        }
        root->right->color = BLACK;
    }
    return true;
}


bool rb_delete_node(RBTreeNode* root, size_t key, size_t* pvalue) {
    RBTreeNode* traverse_path[RB_TREE_MAX_HEIGHT];
    int traverse_path_len = 0;
    RBTreeNode* curr = root->right;
    while(curr && curr->key != key) {
        traverse_path[traverse_path_len++] = curr;
        if(key < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }
    assert(curr && curr->key == key);

    RBTreeNode* successor = curr->right;
    if(curr->right) {
        // sub with successor

        traverse_path[traverse_path_len++] = curr;
        while(successor->left) {
            traverse_path[traverse_path_len++] = successor;
            successor = successor->left;
        }
    } else {
        successor = curr;
    }
    
    // trivial
    assert(
        (successor->color == RED && successor->left == NULL && successor->right == NULL)
        ||
        (successor->color == BLACK && successor->left ==NULL &&
            ( successor->right == NULL 
            || (successor->right->color == RED 
                && successor->right->left == NULL 
                && successor->right->right == NULL))));

    if(curr != successor) {
        *pvalue = curr->value;
        curr->value = successor->value;
        curr->key = successor->key;
        curr = successor;
    }

    // fix
    if(curr->color == RED) {
        assert(traverse_path_len>0);
        RBTreeNode* parent = traverse_path[traverse_path_len-1];
        if(curr == parent->left) {
            parent->left = NULL;
        } else if(curr == parent->right) {
            parent->right = NULL;
        } else {
            assert(false);
        }
        free(curr);
        return true;
    } else if(curr->right) {
        curr->key = curr->right->key;
        curr->value = curr->right->value;
        free(curr->right);
        curr->right = NULL;
        return true;
    } else {
        // fix
        bool is_left_child;
        RBTreeNode* parent;
        if(traverse_path_len==0) {
            root->right = NULL;
            free(curr);
            return true;
        } else {
            parent = traverse_path[--traverse_path_len];
            if(parent->left == curr) {
                parent->left = NULL;
                is_left_child = true;
            } else {
                parent->right = NULL;
                is_left_child = false;
            }
        }

        // do not dereference CURR!
        // while(curr != root->right && curr->color == BLACK) {
        do {
            if(is_left_child) {
                RBTreeNode* sibling = parent->right;
                int sib_color = sibling ? sibling->color : BLACK;
                if(sib_color == RED) {
                    // rotate left
                    assert(sibling!=NULL);
                    assert(parent->color == BLACK);
                    size_t parent_key = parent->key;
                    size_t parent_value = parent->value;
                    parent->right = sibling->right;
                    sibling->right = sibling->left;
                    sibling->left = parent->left;
                    sibling->key = parent_key;
                    sibling->value = parent_value;

                    traverse_path[traverse_path_len++] = parent;
                    parent = sibling;
                    sibling = parent->right;
                }

                assert(sibling != NULL);
                
                if((sibling->left==NULL || sibling->left->color == BLACK)
                    && (sibling->right==NULL || sibling->right->color == BLACK)) {
                    sibling->color = RED;
                    curr = parent;
                } else {

                    if(sibling->right==NULL || sibling->right->color == BLACK) {
                        assert( sibling->left != NULL && sibling->left->color == RED);
                        assert( sibling->left->left != NULL && sibling->left->right != NULL);

                        // rotate right
                        sibling->color = RED;
                        parent->right = sibling->left;
                        sibling->left = sibling->left->right;
                        sibling->left->right = sibling;
                        sibling = parent->right;
                        sibling->color = BLACK;
                    }

                    // rotate right
                    size_t parent_key = parent->key;
                    size_t parent_value = parent->value;
                    parent->key = sibling->key;
                    parent->value = sibling->value;
                    parent->right= sibling->right;
                    parent->right->color = BLACK;
                    sibling->right = sibling->left;
                    sibling->left = parent->left;
                    sibling->key = parent_key;
                    sibling->value = parent_value;
                    // sibling->color = BLACK;
                    parent->left = sibling;

                    curr = root->right;
                    traverse_path_len = 0;
                }
            } else {
                RBTreeNode* sibling = parent->left;
                int sib_color = sibling ? sibling->color : BLACK;
                if(sib_color == RED) {
                    // rotate right
                    assert(sibling != NULL);
                    assert(parent->color == BLACK);
                    size_t parent_key = parent->key;
                    size_t parent_value = parent->value;
                    parent->left = sibling->left;
                    sibling->left = sibling->right;
                    sibling->right = parent->right;
                    sibling->key = parent_key;
                    sibling->value = parent_value;

                    traverse_path[traverse_path_len++] = parent;
                    parent = sibling;
                    sibling = parent->left;
                }

                assert(sibling != NULL);

                if((sibling->left == NULL || sibling->left->color == BLACK)
                    && (sibling->right == NULL || sibling->right->color == BLACK)) {
                    sibling->color = RED;
                    curr = parent;
                } else {

                    if(sibling->left == NULL || sibling->left->color == BLACK) {
                        assert(sibling->right && sibling->right->color == RED);
                        assert(sibling->right->left && sibling->right->right);

                        sibling->color = RED;
                        parent->left = sibling->right;
                        sibling->right = sibling->right->left;
                        sibling->right->left = sibling;
                        sibling = parent->left;
                        sibling->color = BLACK;
                    }

                    size_t parent_key = parent->key;
                    size_t parent_value = parent->value;
                    parent->key = sibling->key;
                    parent->value = sibling->value;
                    parent->left = sibling->left;
                    parent->left->color = BLACK;
                    sibling->left = sibling->right;
                    sibling->right = parent->right;
                    sibling->key = parent_key;
                    sibling->value = parent_value;
                    // sibling->color = BLACK;
                    parent->right = sibling;

                    curr = root->right;
                    traverse_path_len = 0;
                }
            }
            parent = traverse_path[--traverse_path_len];
            is_left_child = (curr == parent->left);
        }while(curr != root->right && curr->color == BLACK);
    }

    if(curr) curr->color = BLACK;

    return true;
}

void rb_print_node(RBTreeNode* node, int level) {
    if(!node) {
        printf("NODE(EMPTY)\n");
        return;
    }

    // level, key, color, value, addr)
    printf("NODE(%d, %llu, %d, %llu, %p)\n",
        level,
        node->key,
        node->color,
        node->value,
        node);
    rb_print_node(node->left, level+1);
    rb_print_node(node->right, level+1);
}

void rb_print_tree(RBTreeNode* root) {
    printf("---------------------------\n");
    printf("TREE(%p)(%p)\n", root, root->right);
    printf("NODE(level, key, color, value, addr)\n");
    rb_print_node(root->right, 0);
}

// thread unsafe
int target_len = -1;
RBTreeNode* root = NULL;
void rb_check_tree(RBTreeNode* node, bool init = true, 
    int len = 0, size_t range_begin = 0, size_t range_end = ~(0llu)) {

    if(init) {
        target_len = -1;
        root = node;
        rb_check_tree(node->right, false, 0);
    } else {
        if(node == NULL) {
            if(target_len<0)
                target_len = len;
            else{
                assert(target_len == len);
            }
        } else {
            int curr_path_len = (node->color==BLACK) ? len + 1 : len;
            assert(range_begin <= node->key && node->key < range_end);
            if(node->color == RED) {
                assert(node->left == NULL || node->left->color == BLACK);
                assert(node->right == NULL || node->right->color == BLACK);
            }
            rb_check_tree(node->left, false, curr_path_len, range_begin, node->key);
            rb_check_tree(node->right, false, curr_path_len, node->key+1, range_end);
        }
    }
}

#endif // __RB_TREE_HEADER__
