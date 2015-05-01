#include <string>
#include <iostream>
#include <cassert>
#include <cstdio>

namespace suffix_tree_ukkonens {

using std::cout;
using std::endl;
using std::printf;
using std::memcpy;
using std::string;

int const CAPACITY_MAX = (0x1)<<(sizeof(int)*8-2);
int const STRING_END = CAPACITY_MAX + 1;

enum {
    OK,
    NOMEM,
    ERROR,
};

class TreeNodeList {
public:
    size_t begin;
    size_t end;
    char begin_char;
    TreeNodeList* suffix_link;
    TreeNodeList* next_sib;
    TreeNodeList* children;

    TreeNodeList(size_t b, size_t e, size_t bc) : 
            begin(b), end(e), begin_char(bc), suffix_link(nullptr), next_sib(nullptr), children(nullptr) {}
    
    TreeNodeList* find(char target) {
        TreeNodeList* curr = children;
        while(curr && curr->begin_char < target) {
            curr = curr->next_sib;
        }
        if(curr && curr->begin_char == target)
            return curr;
        return nullptr;
    }

    TreeNodeList* insert(TreeNodeList* child) {
        TreeNodeList* prev = nullptr;
        TreeNodeList* curr = children;
        while(curr && curr->begin_char < child->begin_char) {
            prev = curr;
            curr = curr->next_sib;
        }
        if(curr && curr->begin_char == child->begin_char) {
            return nullptr;
        }
        child->next_sib = curr;
        if(prev)
            prev->next_sib = child;
        else
            children = child;
        return child;
    }

    TreeNodeList* split_child(char child_id, TreeNodeList* hint, size_t tail_begin, char tail_begin_char) {
        TreeNodeList* prev = nullptr;
        TreeNodeList* curr = children;
        while(curr && curr->begin_char < child_id) {
            prev = curr;
            curr = curr->next_sib;
        }
        assert(curr && curr->begin_char == child_id);
        assert(!hint || hint == curr);
        assert(tail_begin>curr->begin);

        TreeNodeList* new_head = new TreeNodeList(curr->begin, tail_begin-1, curr->begin_char);
        new_head->next_sib = curr->next_sib;
        curr->next_sib = nullptr;
        curr->begin = tail_begin;
        curr->begin_char = tail_begin_char;
        auto ret = new_head->insert(curr);
        assert(ret == curr);
        if(prev)
            prev->next_sib = new_head;
        else 
            children = new_head;
        return new_head;
    }
};

template<typename TreeNode>
class SuffixTree {
public:
    SuffixTree() : buffer(nullptr), capacity(0), len(0), root(0,0,0) {
        capacity = 512;
        buffer = new char[capacity];
        active_node = &root;
        active_edge = 0;
        active_length = 0;
    }

    int append_str(const char* str) {
        int str_len = strlen(str);
        int total_len = str_len + len;
        if(total_len > CAPACITY_MAX) return NOMEM;
        if(total_len > capacity) __enlarge(total_len);
        memcpy(const_cast<char*>(buffer)+len, str, str_len);

        while(len < total_len) {
            //printf("insert pos:%d char%c\n", len, buffer[len]);
            __append_char(buffer[len], len);
            len += 1;
            //print_tree();
        }
        return OK;
    }

    int search_pattern(const char* str) {
        TreeNode* prev_level = &root;
        const char* str_curr = str;
        while(*str_curr) {
            TreeNode* cmp_node = prev_level->find(*str_curr);
            if(!cmp_node) return -1;
            int idx = cmp_node->begin;
            int end = len < cmp_node->end ? len : cmp_node->end;
            while(*str_curr && idx <= end && *str_curr==buffer[idx]) {
                ++str_curr;
                ++idx;
            }
            if(!*str_curr)
                return idx - (str_curr - str);
            if(idx<=end)
                return -1;
            prev_level = cmp_node;
        }
        assert("should not arrive here!");
    }

    int search_suffix(const char* str) {
        assert(false);
        return ERROR;
    }

    void print_tree() {
        __print_tree(root.children, 0);
    }
private:
    void __append_char(char target, size_t pos) {
        assert(active_length >= 0);
        TreeNode* prev_split = &root;
        TreeNode* explicit_state = test_and_split(target);
        while(explicit_state) {
            TreeNode* new_transition = new TreeNode(pos, STRING_END, target);
            auto ret = explicit_state->insert(new_transition);
            assert(ret == new_transition);
            if(prev_split!=&root) prev_split->suffix_link = explicit_state;
            prev_split = explicit_state;
            suffix_function(pos);
            canonize(pos);
            explicit_state = test_and_split(target);
        }
        if(prev_split!=&root) prev_split->suffix_link = active_node;
        active_length += 1;
        if(active_length==1) {
            active_edge = target;
        }
        canonize(pos+1);
    }
    TreeNode* test_and_split(char target) {
        if(active_length < 0) return nullptr;

        if(active_length == 0) {
            auto ret = active_node->find(target);
            if(ret) return nullptr;
            else return active_node;
        }

        auto active_edge_trans = active_node->find(active_edge);
        assert(active_edge_trans &&  active_edge_trans->begin_char == active_edge);

        size_t cmp_pos = active_edge_trans->begin + active_length;
        char cmp_char = buffer[cmp_pos];
        assert(active_edge_trans->begin < cmp_pos && cmp_pos <= active_edge_trans->end);

        if(cmp_char == target)
            return nullptr;

        return active_node->split_child(active_edge, active_edge_trans, cmp_pos, cmp_char);
    }
    void canonize(size_t target_pos) {
        if(active_length<=0) return;

        auto active_edge_trans = active_node->find(active_edge);
        assert(active_edge_trans && active_edge_trans->begin_char == active_edge);

        size_t cmp_pos = active_edge_trans->begin + active_length;
        while(active_length > 0 && cmp_pos > active_edge_trans->end) {
            if(cmp_pos==active_edge_trans->end+1) {
                active_node = active_edge_trans;
                active_edge = 0;
                active_length = 0;
            } else {
                active_node = active_edge_trans;
                active_length -= (active_edge_trans->end - active_edge_trans->begin + 1);
                active_edge = buffer[target_pos-active_length];
                active_edge_trans = active_node->find(active_edge);
                assert(active_edge_trans);
                cmp_pos = active_edge_trans->begin + active_length;
            }
        }
    }
    void suffix_function(size_t target_pos) {
        TreeNodeList* suffix = active_node->suffix_link;
        if(suffix) {
            active_node = suffix;
            return;
        }

        if(active_node!=&root) {
            active_node = &root;
            return;
        }

        active_length -= 1;
        if(active_length>0) {
            auto active_edge_trans = active_node->find(active_edge);
            assert(active_edge_trans && active_edge_trans->begin_char == active_edge);
            active_edge = buffer[target_pos-active_length];
        } else {
            active_edge = 0;
        }
    }


    void __print_tree(TreeNode* curr, int level) {
        if(level==0) {
            cout << string(16, '-') << endl;
            printf("active_node:%p, active_edge:%c, active_length:%d\n",
                active_node, active_edge, active_length);
        }
        while(curr) {
            cout << string(level*2, ' ');
            printf("(node:%p, level:%d, begin:%ld, end:%ld, suffix:%p, char:",
                curr, level, curr->begin, curr->end, curr->suffix_link);
            int str_begin = curr->begin;
            int str_end = curr->end;
            if(str_end==STRING_END) str_end = len -1;
            cout << string(buffer+str_begin, str_end-str_begin+1) << ")" << endl;
            __print_tree(curr->children, level+1);
            curr = curr->next_sib;
        }
        if(level==0) {
            cout << string(20, '-') << endl;
            //(const_cast<char*>(buffer))[len] = 0;
            //cout << buffer << endl;
        }
    }

    void __enlarge(size_t hint) {
        while(capacity<hint && capacity < CAPACITY_MAX) capacity = capacity << 1;
        char* new_buffer = new char[capacity];
        memcpy(new_buffer, buffer, len);
        delete[] buffer;
        buffer = new_buffer;
    }
private:
    const char* buffer;
    size_t      capacity;
    size_t      len;

    TreeNode    root;

    TreeNode*   active_node;
    char        active_edge;
    int         active_length;
};

} // end namespace suffix_tree_ukkonenes
