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

struct TreeNode_List {
    // [begin, end]
    size_t begin;
    size_t end;
    size_t suffix_match_shift;    // s[begin+suffix_match_shift] == suffix last char
    char suffix_end_ts;
    char begin_char;
    TreeNode_List* next_sib;
    TreeNode_List* children;
    TreeNode_List* suffix_link;


    TreeNode_List(size_t b, size_t e, size_t bc) :
        begin(b), end(e), suffix_match_shift(0), suffix_end_ts(0), begin_char(bc),
        next_sib(nullptr), children(nullptr), suffix_link(nullptr)
    {}

    TreeNode_List* find(char target) {
        TreeNode_List* curr = children;
        while(curr && curr->begin_char < target)
            curr = curr->next_sib;
        if(!curr || curr->begin_char!=target)
            return nullptr;
        return curr;
    }

    TreeNode_List* insert(TreeNode_List* child) {
        TreeNode_List* curr = children;
        TreeNode_List* prev = nullptr;
        char target = child->begin_char;
        while(curr && curr->begin_char < target) {
            prev = curr;
            curr = curr->next_sib;
        }
        if(curr && curr->begin_char==target) {
            return nullptr;
        }
        child->next_sib = curr;
        if(prev)
            prev->next_sib = child;
        else 
            children = child;
        return child;
    }
    TreeNode_List* split_on(size_t pos, char pos_char) {
        assert(pos>this->begin && pos<=this->end);
        TreeNode_List* tail = new TreeNode_List(pos, this->end, pos_char);
        tail->children = this->children;
        this->children = nullptr;
        this->end = pos - 1;
        TreeNode_List* ret = this->insert(tail);
        assert(ret==tail);
        return this;
    }
};

enum {
    OK,
    NOMEM,
    ERROR,
};

template<typename TreeNode>
class SuffixTree {

    const char* buffer;
    size_t capacity;
    int len;

    TreeNode root;
    
    TreeNode*   active_node;
    char        active_edge;
    TreeNode*   active_edge_buffer;
    int         active_length;

public:
    SuffixTree() : buffer(nullptr), capacity(0), len(0), root(0,0,0) {
        capacity = 512;
        buffer = new char[capacity];
        active_node = &root;
        active_edge = 0;
        active_edge_buffer = nullptr;
        active_length = 0;
    }

    int append_str(const char* str) {
        int str_len = strlen(str);
        int origin_buffer_len = len;
        int total_len = str_len + len;
        if(total_len > CAPACITY_MAX) return NOMEM;
        if(total_len > capacity) __enlarge(total_len);
        memcpy(const_cast<char*>(buffer)+len, str, str_len);

        for(int i=origin_buffer_len; i<total_len; ++i) {
            len = i + 1;
            printf("insert pos:%d char:%c\n", i, str[i]);
            __append_char(buffer[i], i);
            print_tree();
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
            while(*str_curr && idx<=end && *str_curr==buffer[idx]) {
                ++str_curr;
                ++idx;
            }
            if(!*str_curr) {
                return idx - (str_curr - str);
            }
            if(idx<=end) {
                return -1;
            }
            prev_level = cmp_node;
        }
        return -1;
    }

    int search_suffix(const char* str) {
        return -1;
    }

    void print_tree() {
        __print_tree(root.children, 0);
    }
private:
    void __print_tree(TreeNode* root, int level) {
        if(level==0) {
            cout << string(8, '-') << root << string(8,'-') << endl;
            printf("active_node:%p, active_edge:%c, active_edge_buffer:%p, active_length:%d\n",
                active_node, active_edge, active_edge_buffer, active_length);
        }
        while(root!=nullptr) {
            cout << string(level*2, ' ');
            printf("(node:%p, level:%d, begin:%ld, end:%ld, suffix:%p, char:",
                root, level, root->begin, root->end, root->suffix_link);
            int begin = root->begin;
            int end = root->end;
            if(end==STRING_END) end = len -1;
            cout << string(buffer+begin, end-begin+1) << ")" << endl;
            __print_tree(root->children, level+1);
            root = root->next_sib;
        }
        if(level==0) {
            cout << string(20, '-') << endl;
            //(const_cast<char*>(buffer))[len] = 0;
            //cout << buffer << endl;
        }
    }
    void __append_char(char target, size_t pos) {
        assert(active_length>=-1);
        TreeNode* prev_split = nullptr;
        TreeNode* explicit_state = test_and_cut_tail(target);
        while(explicit_state) {
            assert(active_length>=-1);
            TreeNode* new_transition = new TreeNode(pos, STRING_END, target);
            auto ret = explicit_state->insert(new_transition);
            assert(ret==new_transition);
            if(prev_split!=nullptr && explicit_state!=&root) {
                assert(prev_split != explicit_state);
                prev_split->suffix_link = explicit_state;
            }
            prev_split = explicit_state;
            suffix_function(pos);
            if(active_length<0) break;
            canonize(pos);
            explicit_state = test_and_cut_tail(target);
        }
        //if(prev_split!=&root) prev_split->suffix_link = active_node;
        if(active_length==-1) {
            active_edge = 0;
            active_edge_buffer = nullptr;
        } else if(active_length==0) {
            active_edge = target;
            active_edge_buffer = nullptr;
        }
        active_length += 1;
        canonize(pos+1);
    }
    // ret == nullptr:  is end_point==true, finish;
    // ret != nullptr:  create new transition g(ret, [target_pos, #]), end_point==false.
    TreeNode* test_and_cut_tail(char target) {
        assert(active_length>=0);

        if(active_length==0) {
            auto ret = active_node->find(target);
            if(ret) return nullptr;
            else return active_node;
        }

        if(!active_edge_buffer)
            active_edge_buffer = active_node->find(active_edge);

        assert(active_edge != 0);
        assert(active_edge_buffer==nullptr || active_edge_buffer->begin_char == active_edge);
        assert(active_edge_buffer);

        size_t cmp_pos = active_edge_buffer->begin + active_length;
        char cmp_char = buffer[cmp_pos];
        assert(active_edge_buffer->begin < cmp_pos && cmp_pos <= active_edge_buffer->end);

        if(cmp_char == target)
            return nullptr;
        active_edge_buffer->split_on(cmp_pos, cmp_char);

        return active_edge_buffer;
    }
    void canonize(size_t target_pos) {
        if(active_length <= 0) return;

        if(!active_edge_buffer)
            active_edge_buffer = active_node->find(active_edge);
        assert(active_edge_buffer);
        size_t cmp_pos = active_edge_buffer->begin + active_length;
        while(active_length > 0 && cmp_pos > active_edge_buffer->end) {
            if(cmp_pos==active_edge_buffer->end+1) {
                active_node = active_edge_buffer;
                active_edge = 0;
                active_edge_buffer = nullptr;
                active_length = 0;
            } else {
                active_node = active_edge_buffer;
                active_length -= (active_edge_buffer->end - active_edge_buffer->begin + 1);
                active_edge = buffer[target_pos-active_length];
                active_edge_buffer = active_node->find(active_edge);
                assert(active_edge_buffer);
                cmp_pos = active_edge_buffer->begin + active_length;
            }
        }
    }
    void suffix_function(size_t target_pos) {
        if(active_node->suffix_link) {
            active_node = active_node->suffix_link;
            active_edge_buffer = nullptr;
            return;
        }

        // active_node->suffix_link == nullptr
        if(active_node!=&root) {
            active_node = &root;
            active_edge_buffer = nullptr;
            return;
        }

        assert(active_node==&root);
        if(active_node==&root)
            active_length -= 1;
        
        if(active_length > 0) {
            if(!active_edge_buffer)
                active_edge_buffer = active_node->find(active_edge);
            assert(active_edge_buffer);
            active_edge = buffer[target_pos-active_length];
            active_edge_buffer = nullptr;
        } else {
            active_edge = 0;
            active_edge_buffer = nullptr;
        }
        return;
    }
    void __enlarge(size_t hint) {
        while(capacity<hint && capacity<CAPACITY_MAX) capacity = capacity << 1;
        char* new_buffer = new char[capacity];
        memcpy(new_buffer, buffer, len);
        delete[] buffer;
        buffer = new_buffer;
    }
};

} // end namespace suffix_tree_ukkonenes
