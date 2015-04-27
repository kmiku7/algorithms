#ifndef __B_TREE_HEADER__
#define __B_TREE_HEADER__


#include <cstring>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <climits>
#include <limits>

using std::memmove;
using std::memcpy;

#define PAGE_SIZE   4096
//#define T           85
#define T           10
#define BNODE_KEY_SIZE  (2*T-1)
#define BNODE_CHILDREN_SIZE (2*T)

struct BTreeNode {
    // usable: 16 + 8 + 8
    size_t count;
    bool is_leaf;

    size_t keys[BNODE_KEY_SIZE];
    size_t values[BNODE_KEY_SIZE];
    size_t children[BNODE_CHILDREN_SIZE];

    BTreeNode() : count(0), is_leaf(true) {}
};

// utility
// find the first item that great or equal to target
size_t search_lower_bound(size_t* values, size_t len, size_t target) {
    size_t begin = 0;
    size_t end = len;
    while(begin<end) {
        size_t mid = begin + (end - begin) / 2;
        if(values[mid] < target)
            begin = mid + 1;
         else
            end = mid;
    }
    return begin;
}


// simiulation
// sim_new_node()返回的节点相当于掉用过sim_get_node()，可以直接使用，
// 并需要调用sim_release_node()释放。
void sim_new_node(BTreeNode** page_addr, size_t* node_id) {
    assert(page_addr!=nullptr && node_id!=nullptr);
    BTreeNode* node = new BTreeNode();
    *page_addr = node;
    *node_id = reinterpret_cast<size_t>(node);
}

BTreeNode* sim_get_node(size_t node_id) {
    return reinterpret_cast<BTreeNode*>(node_id);
}

void sim_release_node(size_t node_id, BTreeNode* mem_addr) {
    assert(node_id==reinterpret_cast<size_t>(mem_addr));
    assert( (node_id==0 && mem_addr==nullptr)
            || (node_id!=0 && mem_addr!=nullptr));
}

// sim_delete_node()只能删除没有使用中的节点，
// 即需要先sim_realse_node()。
void sim_delete_node(size_t node_id) {
    BTreeNode* node_addr = reinterpret_cast<BTreeNode*>(node_id);
    delete node_addr;
}

// operation
size_t b_tree_create() {
    BTreeNode* node_addr;
    size_t node_id;
    sim_new_node(&node_addr, &node_id);
    sim_release_node(node_id, node_addr);
    assert(node_addr!=nullptr && node_id!=0);
    assert(node_addr->count==0 && node_addr->is_leaf==true);
    return node_id;
}


void b_tree_split_child(BTreeNode* node_addr, int pos) {
    size_t child_id = node_addr->children[pos];
    BTreeNode* child_addr = sim_get_node(child_id);

    assert(child_addr->count == BNODE_KEY_SIZE);

    BTreeNode* sib_addr;
    size_t sib_id;
    sim_new_node(&sib_addr, &sib_id);
    sib_addr->is_leaf = child_addr->is_leaf;
    sib_addr->count = T - 1;
    memcpy(sib_addr->keys, child_addr->keys+T, sizeof(size_t)*(T-1));
    memcpy(sib_addr->values, child_addr->values+T, sizeof(size_t)*(T-1));
    if(!child_addr->is_leaf) {
        memcpy(sib_addr->children, child_addr->children+T, sizeof(size_t)*T);
    }
    child_addr->count = T - 1;
    if(pos<node_addr->count) {
        memmove(node_addr->keys+pos+1, node_addr->keys+pos, (node_addr->count-pos)*sizeof(size_t));
        memmove(node_addr->values+pos+1, node_addr->values+pos, (node_addr->count-pos)*sizeof(size_t));
        memmove(node_addr->children+pos+2, node_addr->children+pos+1, (node_addr->count-pos)*sizeof(size_t));
    }
    node_addr->keys[pos] = child_addr->keys[T-1];
    node_addr->values[pos] = child_addr->values[T-1];
    node_addr->children[pos+1] = sib_id;
    node_addr->count += 1;

    sim_release_node(sib_id, sib_addr);
    sim_release_node(child_id, child_addr);
}

// insert : True
// update : False
bool b_tree_insert(size_t* p_node_id, size_t key, size_t value) {
    size_t node_id = *p_node_id;
    BTreeNode* node_addr = sim_get_node(node_id);
    
    // 根节点特殊处理。
    if(node_addr->count==BNODE_KEY_SIZE) {
        BTreeNode* new_addr;
        size_t new_id;
        sim_new_node(&new_addr, &new_id);
        new_addr->children[0] = node_id;
        new_addr->is_leaf = false;
        b_tree_split_child(new_addr, 0);
        sim_release_node(node_id, node_addr);

        node_id = *p_node_id = new_id;
        node_addr= new_addr;
    }

    size_t pos;
    do{
        node_addr = sim_get_node(node_id);
        pos = search_lower_bound(node_addr->keys, node_addr->count, key);
        if(node_addr->is_leaf || pos<node_addr->count && node_addr->keys[pos]==key) {
            break;
        }
        size_t next_id = node_addr->children[pos];
        BTreeNode* next_addr = sim_get_node(next_id);
        // 下降前确保该子节点的keys数没满。
        // 根据算法流程，当前节点总是没有满的。
        if(next_addr->count==BNODE_KEY_SIZE) {
            b_tree_split_child(node_addr, pos);
            assert(pos<node_addr->count);
            // children[pos]节点分裂后的keys[pos]大小会变，且是变小。
            // split之前： node_addr->keys[pos] > key
            // split之后： keys[pos] == key || keys[pos] < key
            // 
            // 更好的处理办法就是split之后，下次还是循环当前层，而不直接下降。
            if(node_addr->keys[pos]<key) {
                pos += 1;
                assert(pos>=node_addr->count || key<=node_addr->keys[pos]);
                if(pos>=node_addr->count || key<node_addr->keys[pos]) {
                    size_t sib_id = node_addr->children[pos];
                    BTreeNode* sib_addr = sim_get_node(sib_id);
                    sim_release_node(next_id, next_addr);
                    next_id = sib_id;
                    next_addr = sib_addr;
                } else { // key==node_addr->keys[pos]
                    sim_release_node(next_id, next_addr);
                    break;
                }
            } else if(node_addr->keys[pos]==key) {
                sim_release_node(next_id, next_addr);
                break;
            }
        }
        sim_release_node(node_id, node_addr);
        node_id = next_id;
        node_addr = next_addr;
    }while(node_id);

    // 走到这里有三种可能：
    //  1 叶节点，无重复
    //  2 叶节点，有重复
    //  3 内部节点，必是重复
    assert(node_addr->is_leaf || (!node_addr->is_leaf && node_addr->keys[pos]==key));
    bool is_insert = true;
    if(pos<node_addr->count && node_addr->keys[pos]==key) {
        node_addr->values[pos] = value;
        is_insert = false;
    } else {
        // 新元素总是插入在叶节点，所以不需要考虑children数组的拷贝移动。
        if(pos<node_addr->count) {
            memmove(node_addr->keys+pos+1, node_addr->keys+pos, (node_addr->count-pos)*sizeof(size_t));
            memmove(node_addr->values+pos+1, node_addr->values+pos, (node_addr->count-pos)*sizeof(size_t));
        }
        node_addr->count += 1;
        node_addr->keys[pos] = key;
        node_addr->values[pos] = value;
    }
    sim_release_node(node_id, node_addr);
    return is_insert;
}

void __b_tree_borrow_from_left_sib(BTreeNode* parent, size_t child_pos, BTreeNode* child_addr, BTreeNode* left_sib) {
    assert(parent && child_addr && left_sib);
    assert(child_pos>0 && child_pos<=parent->count);
    assert(left_sib->count >= T);

    size_t left_sib_pos = child_pos - 1;

    memmove(child_addr->keys+1, child_addr->keys, child_addr->count*sizeof(size_t));
    memmove(child_addr->values+1, child_addr->values, child_addr->count*sizeof(size_t));
    if(!child_addr->is_leaf) {
        memmove(child_addr->children+1, child_addr->children, (child_addr->count+1)*sizeof(size_t));
    }

    child_addr->keys[0] = parent->keys[left_sib_pos];
    child_addr->values[0] = parent->values[left_sib_pos];
    if(!child_addr->is_leaf) {
        child_addr->children[0] = left_sib->children[left_sib->count];
    }
    child_addr->count += 1;

    parent->keys[left_sib_pos] = left_sib->keys[left_sib->count-1];
    parent->values[left_sib_pos] = left_sib->values[left_sib->count-1];

    left_sib->count -= 1;
}

void __b_tree_borrow_from_right_sib(BTreeNode* parent, size_t child_pos, BTreeNode* child_addr, BTreeNode* right_sib) {
    assert(parent && child_addr && right_sib);
    assert(child_pos>=0 && child_pos<parent->count);
    assert(right_sib->count >= T);

    child_addr->keys[child_addr->count] = parent->keys[child_pos];
    child_addr->values[child_addr->count] = parent->values[child_pos];
    if(!child_addr->is_leaf) {
        child_addr->children[child_addr->count+1] = right_sib->children[0];
    }
    child_addr->count += 1;

    parent->keys[child_pos] = right_sib->keys[0];
    parent->values[child_pos] = right_sib->values[0];

    memmove(right_sib->keys, right_sib->keys+1, (right_sib->count-1)*sizeof(size_t));
    memmove(right_sib->values, right_sib->values+1, (right_sib->count-1)*sizeof(size_t));
    memmove(right_sib->children, right_sib->children+1, right_sib->count*sizeof(size_t));
    right_sib->count -= 1;
}

void __b_tree_merge_with_right_sib(BTreeNode* parent, size_t child_pos, BTreeNode* child_addr, BTreeNode* right_sib_addr) {
    assert(child_addr!=nullptr && right_sib_addr!=nullptr);
    assert(child_addr->count + right_sib_addr->count + 1 <= BNODE_KEY_SIZE);
    assert(child_pos < parent->count);
    assert(child_addr->is_leaf == right_sib_addr->is_leaf);

    child_addr->keys[child_addr->count] = parent->keys[child_pos];
    child_addr->values[child_addr->count] = parent->values[child_pos];
    child_addr->count += 1;
    memcpy(child_addr->keys+child_addr->count, right_sib_addr->keys, right_sib_addr->count*sizeof(size_t));
    memcpy(child_addr->values+child_addr->count, right_sib_addr->values, right_sib_addr->count*sizeof(size_t));
    if(!child_addr->is_leaf) {
        memcpy(child_addr->children+child_addr->count, right_sib_addr->children, (right_sib_addr->count+1)*sizeof(size_t));
    }
    child_addr->count += right_sib_addr->count;

    right_sib_addr->count = 0;

    memmove(parent->keys + child_pos, parent->keys + child_pos + 1, (parent->count - child_pos - 1)*sizeof(size_t));
    memmove(parent->values + child_pos, parent->values + child_pos + 1, (parent->count - child_pos - 1)*sizeof(size_t));
    memmove(parent->children + child_pos + 1, parent->children + child_pos + 2, (parent->count - child_pos - 1)*sizeof(size_t));
    parent->count -= 1;
}

void __b_tree_find_predecessor(BTreeNode* node, size_t pos, size_t* p_key, size_t* p_value) {
    assert(node!=nullptr && p_key!=nullptr && p_value!=nullptr);
    assert(pos>=0 && pos<node->count);
    assert(!node->is_leaf && node->children[pos]!=0);

    size_t node_id = node->children[pos];
    BTreeNode* node_addr = sim_get_node(node_id);
    assert(node_addr);

    while(!node_addr->is_leaf) {
        size_t next_node_id = node_addr->children[node_addr->count];
        sim_release_node(node_id, node_addr);
        node_id = next_node_id;
        node_addr = sim_get_node(node_id);
        assert(node_addr);
    }

    *p_key = node_addr->keys[node_addr->count-1];
    *p_value = node_addr->values[node_addr->count-1];
    sim_release_node(node_id, node_addr);
}

void __b_tree_find_successor(BTreeNode* node, size_t pos, size_t* p_key, size_t* p_value) {
    assert(node!=nullptr && p_key!=nullptr && p_value!=nullptr);
    assert(pos>=0 && pos<node->count);
    assert(node->children[pos]!=0);

    size_t node_id = node->children[pos+1];
    BTreeNode* node_addr = sim_get_node(node_id);
    assert(node_addr);

    while(!node_addr->is_leaf) {
        size_t next_node_id = node_addr->children[0];
        sim_release_node(node_id, node_addr);
        node_id = next_node_id;
        node_addr = sim_get_node(node_id);
        assert(node_addr);
    }

    *p_key = node_addr->keys[0];
    *p_value = node_addr->values[0];
    sim_release_node(node_id, node_addr);
}

// exist : True
bool b_tree_delete(size_t* p_node_id, size_t raw_key, size_t* raw_value) {
    size_t key = raw_key;
    size_t* value = raw_value;
    size_t node_id = *p_node_id;
    BTreeNode* node_addr;
    node_addr = sim_get_node(node_id);
    assert(node_addr!=nullptr);

    bool exists = false;

    do{
        size_t pos = search_lower_bound(node_addr->keys, node_addr->count, key);
        if(!node_addr->is_leaf && (pos>=node_addr->count || node_addr->keys[pos]!=key)) {
            size_t child_id = node_addr->children[pos];
            BTreeNode* child_addr = sim_get_node(child_id);
            if(child_addr->count<=T-1) {
                // move item from right sib
                size_t left_sib_id = 0;
                size_t right_sib_id = 0;
                BTreeNode* left_sib_addr = nullptr;
                BTreeNode* right_sib_addr = nullptr;
                // 3a
                // 判断右兄弟count(keys)是否多于T-1个，如果是则移动一个到当前节点。
                if(pos+1<=node_addr->count) {
                    right_sib_id = node_addr->children[pos+1];
                    right_sib_addr = sim_get_node(right_sib_id);
                    if(right_sib_addr->count>=T) {
                        __b_tree_borrow_from_right_sib(node_addr, pos, child_addr, right_sib_addr);
                    }
                }
                // 3a
                // 同上
                if(child_addr->count<=T-1 && pos>0) {
                    left_sib_id = node_addr->children[pos-1];
                    left_sib_addr =sim_get_node(left_sib_id);
                    if(left_sib_addr->count>=T) {
                        __b_tree_borrow_from_left_sib(node_addr, pos, child_addr, left_sib_addr);
                    }
                }
                if(child_addr->count>=T) goto CASE_3_NEXT_LOOP;


                assert(right_sib_id!=0 || left_sib_id!=0);
                // 3b
                // 如果有左右兄弟节点的话，可能兄弟节点的keys都为T-1，则考虑合并。
                // 合并前父节点只有一个key，则必为根节点，需要替代。
                if(child_addr->count<=T-1 && right_sib_id!=0) {
                    __b_tree_merge_with_right_sib(node_addr, pos, child_addr, right_sib_addr);
                    sim_release_node(right_sib_id, right_sib_addr);
                    sim_delete_node(right_sib_id);
                    right_sib_id = 0;
                    right_sib_addr = nullptr;
                }
                // 3b
                // 同上。
                if(child_addr->count<=T-1 && left_sib_id!=0) {
                    __b_tree_merge_with_right_sib(node_addr, pos-1, left_sib_addr, child_addr);
                    sim_release_node(child_id, child_addr);
                    sim_delete_node(child_id);
                    child_id = left_sib_id;
                    child_addr = left_sib_addr;
                    left_sib_id = 0;
                    left_sib_addr = nullptr;
                }
                assert(child_addr->count>=T);

                // 经过3b的处理后，父节点是根节点的情况下count可能减为0，需要处理。
                if(node_addr->count==0) {
                    assert(node_id==*p_node_id);
                    sim_release_node(node_id, node_addr);
                    sim_delete_node(node_id);
                    node_id = 0;
                    node_addr = nullptr;
                    *p_node_id = child_id;
                }
CASE_3_NEXT_LOOP:
                // 释放page。
                if(left_sib_id) sim_release_node(left_sib_id, left_sib_addr);
                if(right_sib_id) sim_release_node(right_sib_id, right_sib_addr);
            }
            sim_release_node(node_id, node_addr);
            node_id = child_id;
            node_addr = child_addr;
        } else if(node_addr->is_leaf) {
            // 到达叶节点，目标是否存在需要继续判断，无特殊处理逻辑。
            // 直接删，不用判断剩余keys数。
            if(pos<node_addr->count && node_addr->keys[pos]==key) {
                exists = true;
                if(value) *value = node_addr->values[pos];
                memmove(node_addr->keys+pos, node_addr->keys+pos+1, (node_addr->count-pos-1)*sizeof(size_t));
                memmove(node_addr->values+pos, node_addr->values+pos+1, (node_addr->count-pos-1)*sizeof(size_t));
                node_addr->count -= 1;
            } else {
                exists = false;
            }
            break;
        } else { 
            // !node_addr->is_leaf && (pos<node_addr->count && node_addr->keys[pos]==key)
            // 目标元素处于非叶节点，已经定位位置。
            exists = true;
            if(value) *value = node_addr->values[pos];

            size_t left_child_id = 0;
            size_t right_child_id = 0;

            BTreeNode* left_child_addr = nullptr;
            BTreeNode* right_child_addr = nullptr;
            bool deleted = false;
    

            // 2a
            // 使用前驱替换。
            left_child_id = node_addr->children[pos];
            left_child_addr = sim_get_node(left_child_id);
            if(left_child_addr->count>=T) {
                size_t sub_key;
                size_t sub_value;
                __b_tree_find_predecessor(node_addr, pos, &sub_key, &sub_value);
                if(value) *value = node_addr->values[pos];
                node_addr->keys[pos] = sub_key;
                node_addr->values[pos] = sub_value;

                // here.
                key = sub_key;
                value = nullptr;
                sim_release_node(node_id, node_addr);
                node_addr = left_child_addr;
                node_id = left_child_id;

                continue;
            }

            // 2b
            // 使用后继替换。
            right_child_id = node_addr->children[pos+1];
            right_child_addr = sim_get_node(right_child_id);
            if(right_child_addr->count>=T) {
                size_t sub_key;
                size_t sub_value;
                __b_tree_find_successor(node_addr, pos, &sub_key, &sub_value);
                if(value) *value = node_addr->values[pos];
                node_addr->keys[pos] = sub_key;
                node_addr->values[pos] = sub_value;

                // here.
                key = sub_key;
                value = nullptr;
                sim_release_node(node_id, node_addr);
                node_addr = right_child_addr;
                node_id = right_child_id;

                sim_release_node(left_child_id, left_child_addr);
                continue;
            }

            // 2c
            // 左右子节点合并。
            assert(!deleted);
            assert(left_child_id!=0 && right_child_id!=0);
            // delete & merge
            __b_tree_merge_with_right_sib(node_addr, pos, left_child_addr, right_child_addr);
            sim_release_node(right_child_id, right_child_addr);
            sim_delete_node(right_child_id);
            right_child_id = 0;
            right_child_addr = nullptr;
            if(node_addr->count==0) {
                assert(node_id==*p_node_id);
                sim_release_node(node_id, node_addr);
                sim_delete_node(node_id);
                *p_node_id = left_child_id;
            } else {
                sim_release_node(node_id, node_addr);
            }
            node_addr = left_child_addr;
            node_id = left_child_id;
        }
    }while(true);

    sim_release_node(node_id, node_addr);
    return exists;
}

// exist : True
bool b_tree_search(size_t node_id, size_t key, size_t* value) {
    if(node_id==0) return false;
    BTreeNode* node_addr = sim_get_node(node_id);
    size_t pos;
    do{
        pos = search_lower_bound(node_addr->keys, node_addr->count, key);
        if(node_addr->is_leaf || pos<node_addr->count && node_addr->keys[pos]==key)
            break;
        size_t child_id = node_addr->children[pos];
        sim_release_node(node_id, node_addr);
        node_id = child_id;
        node_addr = sim_get_node(node_id);
    }while(node_id);
    bool exist = false;
    if(pos<node_addr->count && node_addr->keys[pos]==key) {
        exist = true;
        if(value) *value = node_addr->values[pos];
    }
    sim_release_node(node_id, node_addr);
    return exist;
}

void test_print_btree(size_t root_id, int level=0, bool is_root=true) {
    using std::cout;
    using std::endl;
    using std::string;
    using std::copy;
    using std::ostream_iterator;

    if(!root_id) return;

    BTreeNode* root = sim_get_node(root_id);
    if(is_root) {
        cout << string(7, '-') << " btree " << string(7, '-') << endl;
        cout << "T: " << T << endl;
    }

    int indent = level;
    cout << string(indent*3, ' ');
    cout << " count: " << root->count;
    cout << " leaf: " << root->is_leaf;
    cout << " key: ";
    copy(root->keys, root->keys+root->count, ostream_iterator<size_t>(cout, ","));
    cout << endl;

    if(!root->is_leaf) {
        for(int i=0; i<=root->count; ++i) {
            test_print_btree(root->children[i], level+1, false);
        }
    }
    sim_release_node(root_id, root);

    if(is_root) {
        cout << string(21, '-') << endl;
    }

}

#define SIZE_T_MIN  0
#define SIZE_T_MAX  (~static_cast<size_t>(0x1))
void check_btree_restrict(size_t node_id, int is_root=true, 
        size_t lower_bound = SIZE_T_MIN,
        size_t upper_bound = SIZE_T_MAX) {

    BTreeNode* node_addr = sim_get_node(node_id);
    // count(keys) belongs to [T-1, 2T-1], except root
    assert( (!is_root && node_addr->count >= T-1 && node_addr->count <=2*T-1)
            ||
            (is_root && ((node_addr->count==0 && node_addr->is_leaf) || (node_addr->count > 0))));

    for(int i=0; i<node_addr->count; ++i) {
        assert(node_addr->keys[i] >= lower_bound && node_addr->keys[i] <= upper_bound);
    }

    // can be stronger.
    assert( (!is_root && !node_addr->is_leaf 
                && node_addr->keys[0]>SIZE_T_MIN 
                && node_addr->keys[node_addr->count-1]<SIZE_T_MAX)
            || is_root
            || node_addr->is_leaf);
             
    if(!node_addr->is_leaf) {
        for(int i=0; i<=node_addr->count; ++i) {
            size_t child_lower_bound = lower_bound;
            size_t child_upper_bound = upper_bound;
            if(i>0)
                child_lower_bound = node_addr->keys[i-1]+1;
            if(i<node_addr->count)
                child_upper_bound = node_addr->keys[i]-1;
            check_btree_restrict(node_addr->children[i], false, child_lower_bound, child_upper_bound);
        }
    }
    sim_release_node(node_id, node_addr);
}
#endif //  __B_TREE_HEADER__
