#include "rbtree.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <climits>

using std::printf;
using std::calloc;
using std::free;

const int RB_TREE_MAX_HEIGHT = 128;

static void __rb_print_node(RBTreeNode* node, int level);
// 不改变节点颜色。
static void __rb_rotate_right(RBTreeNode* node);
static void __rb_rotate_left(RBTreeNode* node);

// root->right指向根元素。
RBTreeNode* rb_create_tree() {
	RBTreeNode* node = static_cast<RBTreeNode*>(calloc(1, sizeof(RBTreeNode)));
	node->color = ROOT;
	node->key = 0;
	return node;
}

void rb_destroy_tree(RBTreeNode* root) {
	while (root) {
		if (root->left == NULL) {
			RBTreeNode* temp = root->right;
			free(root);
			root = temp;
		} else {
			RBTreeNode* precessor = root->left;
			while (precessor->right != NULL) {
				precessor = precessor->right;
			}
			if(precessor->right == NULL) {
				precessor->right = root->right;

				RBTreeNode* temp = root->left;
				free(root);
				root = temp;
			}
		}
	}
}

bool rb_insert_node(RBTreeNode* root, size_t key, size_t value) {
	assert(root != NULL);

	RBTreeNode* path[RB_TREE_MAX_HEIGHT] = {root};
	size_t path_len = 1;

	RBTreeNode* node = root->right;
	while(node) {
		assert(node->key != key);
		path[path_len++] = node;
		if(key < node->key) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	node = static_cast<RBTreeNode*>(calloc(1, sizeof(RBTreeNode)));
	node->key = key;
	node->value = value;
	node->color = RED;

	RBTreeNode* parent = path[--path_len];
	if(key < parent->key) {
		assert(parent->left == NULL);
		parent->left = node;
	} else {
		assert(parent->right == NULL);
		parent->right = node;
	}

	while(parent->color == RED && path_len>1) {
		RBTreeNode* grandpa = path[--path_len];
		assert(grandpa->color == BLACK);
		if(parent == grandpa->left) {
			if(grandpa->right == NULL || grandpa->right->color == BLACK) {
				if(node == parent->right) {
					__rb_rotate_left(parent);
				}
				__rb_rotate_right(grandpa);
				assert(grandpa->color == RED);
				assert(grandpa->right != NULL);
				assert(grandpa->right->color == BLACK);

				grandpa->color = BLACK;
				grandpa->right->color = RED;
				break;
			} else {
				grandpa->right->color = BLACK;
				parent->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
				parent = path[--path_len];
			}
		} else if(parent == grandpa->right) {
			if(grandpa->left == NULL || grandpa->left->color == BLACK) {
				if(node == parent->left) {
					__rb_rotate_right(parent);
				}
				__rb_rotate_left(grandpa);
				assert(grandpa->color = RED);
				assert(grandpa->left != NULL);
				assert(grandpa->left->color == BLACK);

				grandpa->color = BLACK;
				grandpa->left->color = RED;
				break;
			} else {
				grandpa->left->color = BLACK;
				parent->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
				parent = path[--path_len];
			}
		} else {
			assert(false && "invalid path.");
		}
	}

	assert(root->right != NULL);
	assert(root->left == NULL);
	if(root->right->color == RED)
		root->right->color =BLACK;

	return true;
}

bool rb_delete_node(RBTreeNode* root, size_t key, size_t* value) {
	assert(root != NULL);
	assert(value != NULL);

	RBTreeNode* path[RB_TREE_MAX_HEIGHT] = {root};
	size_t path_len = 1;

	RBTreeNode* node = root->right;
	while(node != NULL) {
		path[path_len++] = node;
		if(node->key == key) {
			break;
		}
		if(node->key < key) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	assert(node->key == key && "target node exist");

	RBTreeNode* successor = node->right;
	while(successor) {
		path[path_len++] = successor;
		successor = successor->left;
	}
	successor = path[path_len-1];
	if(successor != node) {
		*value = node->value;
		node->key = successor->key;
		node->value = successor->value;
	} else {
		*value = node->value;
	}

	RBTreeNode* need_delete = path[--path_len];
	assert(need_delete->left == NULL || need_delete->right==NULL);

	assert(
			(need_delete->color == RED && need_delete->left == NULL && need_delete->right == NULL)
				|| (need_delete->color == BLACK
						&& (need_delete->left == NULL
							|| (need_delete->left->color == RED
									&& need_delete->left->left == NULL
									&& need_delete->left->right == NULL))
					&& (need_delete->right == NULL
							|| (need_delete->right->color == RED
									&& need_delete->right->left == NULL
									&& need_delete->right->right == NULL))));

	RBTreeNode* parent_of_need_delete = path[path_len-1];
	if(need_delete->left != NULL) {
		assert(need_delete->color == BLACK && need_delete->left->color == RED);
		// swap color
		need_delete->color = RED;
		need_delete->left->color = BLACK;
	}

	if(need_delete->right != NULL) {
		assert(need_delete->color == BLACK && need_delete->right->color == RED);
		// swap color
		need_delete->color = RED;
		need_delete->right->color = BLACK;
	}

	node = need_delete;
	while(node->color == BLACK && path_len>1) {
		RBTreeNode* parent = path[--path_len];
		if(node == parent->left) {
			assert(parent->right != NULL);
			RBTreeNode* right= parent->right;
			if(right->color == RED) {
				assert(parent->color == BLACK);
				__rb_rotate_left(parent);
				parent->color = BLACK;
				parent->left->color = RED;
				path[path_len++] = parent;
				path[path_len++] = parent->left;
				parent = parent->left;

				// ugly
				if(node == need_delete) {
					parent_of_need_delete = parent;
				}
				assert(node == parent->left);
			} else if((right->right == NULL || right->right->color == BLACK)
					&& (right->left == NULL || right->left->color == BLACK)) {
				right->color = RED;
				node = parent;
			} else {
				if(right->right == NULL || right->right->color == BLACK) {
					assert(right->left != NULL && right->left->color == RED);
					__rb_rotate_right(right);
					right->right->color = RED;
					right->color = BLACK;
				}

				right->color = parent->color;
				parent->color = BLACK;
				right->right->color = BLACK;
				__rb_rotate_left(parent);

				if(node == need_delete) {
					parent_of_need_delete = parent->left;
				}

				node = root->right;
				break;
			}
		} else if(node == parent->right) {
			// more compact
			assert(parent->left != NULL);
			RBTreeNode* left = parent->left;
			if(left->color == RED) {
				parent->color = RED;
				left->color = BLACK;
				__rb_rotate_right(parent);
				path[path_len++] = parent;
				path[path_len++] = left;
				parent = left;

				if(node == need_delete) {
					parent_of_need_delete = parent;
				}
			} else if((left->right == NULL || left->right->color == BLACK)
					&& (left->left == NULL || left->left->color == BLACK)) {
				left->color = RED;
				node = parent;
			} else {
				if(left->left == NULL || left->left->color == BLACK) {
					left->color = RED;
					left->right->color = BLACK;
					__rb_rotate_left(left);
				}

				left->color = parent->color;
				parent->color = BLACK;
				left->left->color = BLACK;
				__rb_rotate_right(parent);

				if(node == need_delete) {
					parent_of_need_delete = parent->right;
				}

				node = root->right;
				break;
			}
		} else {
			assert(false && "invalid path");
		}
	} // end while(node->color == BLACK && path_len>1)

	if(need_delete == parent_of_need_delete->left) {
		parent_of_need_delete->left = need_delete->left ? need_delete->left : need_delete->right;
	} else if(need_delete == parent_of_need_delete->right) {
		parent_of_need_delete->right = need_delete->left ? need_delete->left : need_delete->right;
	} else {
		assert(false && "invalid parent");
	}

	free(need_delete);
	need_delete = NULL;

	if(node)
		node->color = BLACK;

	return true;
}

void rb_print_tree(RBTreeNode* root) {
	assert(root != NULL);

	printf("--------------------\n");
	printf("Tree(%p)(%p)\n", root, root->right);
	printf("Node(level, key, color, value, address)\n");
	__rb_print_node(root->right, 0);
}

int target_len = -1;
RBTreeNode* root = NULL;
void rb_check_tree(RBTreeNode* node, bool init, int len, size_t range_begin, size_t range_end) {

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

void __rb_print_node(RBTreeNode* node, int level) {
	if(!node) {
		printf("NODE(empty)\n");
		return;
	}

	// level, key, color, value, addr)
	printf("NODE(%d, %lu, %d, %lu, %p)\n",
			level,
			node->key,
			node->color,
			node->value,
			node);
	__rb_print_node(node->left, level+1);
	__rb_print_node(node->right, level+1);
}

void __rb_rotate_right(RBTreeNode* node) {
	assert(node != NULL && node->left != NULL);

	RBTreeNode* left = node->left;
	size_t node_key = node->key;
	size_t node_value = node->value;
	int node_color = node->color;

	node->key = left->key;
	node->value = left->value;
	node->color = left->color;
	node->left = left->left;
	left->left = left->right;
	left->right = node->right;
	left->key = node_key;
	left->value = node_value;
	left->color = node_color;
	node->right = left;
}

void __rb_rotate_left(RBTreeNode* node) {
	assert(node != NULL && node->right != NULL);

	RBTreeNode* right = node->right;
	size_t node_key = node->key;
	size_t node_value = node->value;
	int node_color = node->color;

	node->key = right->key;
	node->value = right->value;
	node->color = right->color;
	node->right= right->right;
	right->right = right->left;
	right->left= node->left;
	right->key = node_key;
	right->value = node_value;
	right->color = node_color;
	node->left= right;
}
