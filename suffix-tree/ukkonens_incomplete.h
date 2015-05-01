#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>

namespace suffix_tree_ukkonens {

using std::cout;
using std::endl;
using std::printf;
using std::string;
using std::memcpy;

int const CAPACITY_MAX = (0x1)<<(sizeof(int)*8-2);
int const STRING_END = CAPACITY_MAX+1;

// wrong
struct TreeNode_List {
    // [begin,end]
    size_t begin;
    size_t end;
    size_t suffix_len;
    char begin_char;
    TreeNode_List* suffix_link;
    TreeNode_List* next_sib;
    TreeNode_List* children;

    TreeNode_List(size_t b=0, size_t e=0, char bc='\0') : begin(b), end(e), suffix_len(0), begin_char(bc),
            suffix_link(nullptr), next_sib(nullptr), children(nullptr) {}
    TreeNode_List* lower_bound(char target) {
        TreeNode_List* curr = children;
        while(curr && curr->begin_char < target) curr = curr->next_sib;
        return curr;
    }
    // if exist, return address of existed node;
    // else return child;
    TreeNode_List* insert(TreeNode_List* child) {
        TreeNode_List* prev = nullptr;
        TreeNode_List* curr = children;
        while(curr && curr->begin_char < child->begin_char) {
            prev = curr;
            curr = curr->next_sib;
        }
        if(curr==nullptr && prev==nullptr) {
            this->children = child;
        } else if(curr==nullptr && prev!=nullptr) {
            prev->next_sib = child;
        } else if( curr->begin_char == child->begin_char) {
            return curr;
        } else {
            child->next_sib = curr;
            if(prev) prev->next_sib = child;
            this->children = child;
        }
        return child;
    }
    // a[0, 10] -> this[11, 17] -> c[18, #]
    //          -> d
    // a[0, 10] -> this[11, pos-1] -> this[pos, 17] -> c[18, #]
    //          -> d
    TreeNode_List* split_on(size_t pos, char pos_char) {
        TreeNode_List* second_half = new TreeNode_List(pos, this->end, pos_char);
        second_half->children = this->children;
        this->children = nullptr;
        this->end = pos - 1;
        this->insert(second_half);
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
    
    TreeNode    root;
    size_t      overlap_count; // just for booking
    TreeNode*   overlap_node;
    TreeNode*   overlap_child_buffer;
    int         overlap_length;
    char        overlap_edge;

    
public:
    SuffixTree() : buffer(nullptr), capacity(512), len(0) {
        buffer = new char[capacity];
        overlap_count = 0;
        overlap_node = &root;
        overlap_child_buffer = nullptr;
        overlap_length = 0;
        overlap_edge = '\0';
    }
    int append_str(const char* str) {
        int str_len = strlen(str);
        int total_len = len+str_len;
        if(total_len>CAPACITY_MAX) return NOMEM;
        if(total_len>capacity) __enlarge(total_len);
        memcpy(const_cast<char*>(buffer)+len, str, str_len);
        size_t new_str_begin = len;

        for(int i=0; i<str_len; ++i) {
            __append_char(str[i], len+i);
            printf("insert pos:%d char:%c\n", i, str[i]);
            print_tree();
        }
        len += str_len;
    }

    int search_pattern(const char* str) {
        TreeNode* prev_level = &root;
        const char* str_curr = str;
        while(*str_curr) {
            TreeNode* cmp_node = prev_level->lower_bound(*str_curr);
            if(!cmp_node) return -1;
            int idx = cmp_node->begin;
            while(*str_curr && *str_curr==buffer[idx] && idx<=cmp_node->end) {
                ++str_curr;
                ++idx;
            }
            if(!*str_curr) {
                return idx - (str_curr-str);
            } else if(idx<=cmp_node->end) {
                return -1;
            }
            prev_level = cmp_node;
        }
    }
    int search_suffix(const char* str);

    void print_tree() {
        __print_tree(root.children, 0);
    }

private:
    void __print_tree(TreeNode* root, int level = 0) {
        if(!level) cout << string(10, '-') << endl;
        while(root) {
            printf("(level:%d, begin:%lu, end:%lu, this:%p, suffix:%p)\n", 
                level, root->begin, root->end, root, root->suffix_link);
            __print_tree(root->children, level+1);
            root = root->next_sib;
        }
        if(!level) cout << string(10, '-') << endl;
    }
    void __append_char(char target, int pos) {
        assert(overlap_node!=nullptr);
        if(overlap_child_buffer && 
            overlap_child_buffer->begin + overlap_length == overlap_child_buffer->end + 1) {

            overlap_node = overlap_child_buffer;
            overlap_child_buffer = nullptr;
            overlap_edge = '\0';
            overlap_length = 0;    
        }
        assert(!overlap_child_buffer || 
            overlap_child_buffer->begin + overlap_length <= overlap_child_buffer->end);
        if(overlap_length==0) {
            TreeNode* may_be_match = overlap_node->lower_bound(target);
            if(may_be_match && may_be_match->begin_char == target) {
                overlap_length += 1;
                overlap_edge = target;
                overlap_child_buffer = may_be_match;
            } else {
                TreeNode* list_curr = overlap_node;
                while(list_curr && list_curr!=&root) {
                    TreeNode* new_child = new TreeNode(pos, STRING_END, target);
                    TreeNode* ret = list_curr->insert(new_child);
                    assert(ret==new_child);
                    list_curr = list_curr->suffix_link;
                }

                may_be_match = root.lower_bound(target);
                if(may_be_match && may_be_match->begin_char == target) {
                    overlap_length += 1;
                    overlap_edge = target;
                    overlap_child_buffer = may_be_match;
                } else {
                    TreeNode* new_child = new TreeNode(pos, STRING_END, target);
                    TreeNode* ret = root.insert(new_child);
                    assert(ret==new_child);
                    overlap_node = &root;
                    overlap_edge = '\0';
                    overlap_length = 0;
                }
            }
        } else {
            assert(overlap_length>0 && overlap_child_buffer!=nullptr);


            size_t  cmp_pos = overlap_child_buffer->begin + overlap_length;
            char    cmp_char = buffer[cmp_pos];
            if(cmp_char == target) {
                overlap_length += 1;
            } else {
                // split
                TreeNode* list_curr = overlap_node;
                TreeNode* prev_split = nullptr;
                while(list_curr && list_curr!=&root) {
                    TreeNode* overlap_child = list_curr->lower_bound(overlap_edge);
                    cmp_pos = overlap_child->begin + overlap_length;
                    cmp_char = buffer[cmp_pos];
                    assert(overlap_child!=nullptr);
                    assert(overlap_child->begin<cmp_pos && cmp_pos<=overlap_child->end);
                    overlap_child->split_on(cmp_pos, cmp_char);
                    TreeNode* new_branch = new TreeNode(pos, STRING_END, target);
                    TreeNode* ret = overlap_child->insert(new_branch);
                    assert(ret==new_branch);
                    if(prev_split) prev_split->suffix_link = overlap_child;
                    prev_split = overlap_child;
                    list_curr = list_curr->suffix_link;
                }
                overlap_node = &root;
                while(overlap_length) {
                    TreeNode* overlap_child = root.lower_bound(overlap_edge);
                    assert(overlap_child!=nullptr);
                    assert(overlap_child->begin + overlap_length <= overlap_child->end);
                    cmp_pos = overlap_child->begin + overlap_length;
                    cmp_char = buffer[cmp_pos];
                    if(cmp_char==target) {
                        overlap_length += 1;
                        overlap_child_buffer = overlap_child;
                        break;
                    }
                    overlap_child->split_on(cmp_pos, cmp_char);
                    TreeNode* new_branch = new TreeNode(pos, STRING_END, target);
                    TreeNode* ret = overlap_child->insert(new_branch);
                    assert(ret==new_branch);
                    if(prev_split) prev_split->suffix_link = overlap_child;
                    prev_split = overlap_child;
                    overlap_length -= 1;
                    overlap_edge = buffer[overlap_child->begin+1];
                }
            }
        }
    }
    void __mark_finish();
    void __unmark_finish();

    void __enlarge(size_t hint) {
        while(capacity<hint) capacity = capacity << 1;
        char* new_buffer = new char[capacity];
        memcpy(new_buffer, buffer, len);
        delete[] buffer;
        buffer = new_buffer;
    }
};

} // end namespace ukkonens
