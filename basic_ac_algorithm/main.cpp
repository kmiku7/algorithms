#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

struct TrieNode {
    struct TrieNode* sibling;
    struct TrieNode* children;
    struct TrieNode* prefix;
    int height; // 1, 2, 3 ...
    bool pattern_end;
    char this_char;
    TrieNode() : sibling(nullptr), children(nullptr), prefix(nullptr), height(0), pattern_end(false), this_char(0) {}
};

TrieNode* get_node() {
    return new TrieNode();
}

void release_node(TrieNode* node) {
    delete node;
}

void free_trie(TrieNode* root) {
    vector<TrieNode*> stack;
    stack.push_back(root);
    while(!stack.empty()){
        root = stack.back();
        if(root==nullptr) {
            stack.pop_back();
            continue;
        }
        stack.back() = root->sibling;
        stack.push_back(root->children);
        release_node(root);
    }
}

// assumption: there are no duplicate item and no empty string in strs.
TrieNode* build_trie(const vector<string>& strs) {
    // build root;
    TrieNode* root = nullptr;
    for(auto &str : strs) {
        int len = 0;
        TrieNode** insert_pos = &root;
        TrieNode* parent_node = nullptr;
        for(auto curr_char : str) {
            // find insert pos
            len += 1;
            while(*insert_pos!=nullptr && (*insert_pos)->this_char<curr_char) 
                insert_pos = &(*insert_pos)->sibling;

            parent_node = *insert_pos;
            // new char, insert
            if(*insert_pos==nullptr || (*insert_pos)->this_char>curr_char) {
                parent_node = get_node();
                parent_node->sibling = *insert_pos;
                parent_node->this_char = curr_char;
                parent_node->height = len;
                *insert_pos = parent_node;
            }
            insert_pos = &parent_node->children;
        }
        // no duplicate
        assert(parent_node && parent_node->pattern_end==false);
        parent_node->pattern_end = true;
    }
    return root;
}

TrieNode* build_suffix(const vector<string>& strs) {
    TrieNode* root = nullptr;
    for(auto &str : strs) {
        TrieNode** insert_pos = &root;
        TrieNode* parent_node = nullptr;
        for(int i=0; i<str.size(); ++i) {
            char curr_char = str[str.size()-1-i];
            while(*insert_pos!=nullptr && (*insert_pos)->this_char<curr_char)
                insert_pos = &(*insert_pos)->sibling;

            parent_node = *insert_pos;
            if(*insert_pos==nullptr || (*insert_pos)->this_char>curr_char) {
                parent_node = get_node();
                parent_node->sibling = *insert_pos;
                parent_node->this_char = curr_char;
                parent_node->height = i+1;
                *insert_pos = parent_node;
            }
            insert_pos = &parent_node->children;
        }
        assert(parent_node && parent_node->pattern_end==false);
        parent_node->pattern_end = true;
    }
    return root;
}

TrieNode* list_search(TrieNode* list_head, char target) {
    while(list_head && list_head->this_char!=target)
        list_head = list_head->sibling;
    return list_head;
}

void build_longest_trans_target(TrieNode* root) {
    assert(root!=nullptr);
    vector<TrieNode*> stack;
    TrieNode* current = root;
    while(current) {
        stack.push_back(current);
        current = current->sibling;
    }
    // bfs可以正确传递pattern_end标志,
    // 但是需要使用queue实现。
    while(!stack.empty()){
        current = stack.back();
        stack.pop_back();
        TrieNode* children = current->children;
        while(children) {
            TrieNode* candidate = current->prefix;
            while(candidate && children->prefix==nullptr) {
                children->prefix = list_search(candidate->children, children->this_char);
                candidate = candidate->prefix;
            }
            if(children->prefix==nullptr)
                children->prefix = list_search(root, children->this_char);
            assert(children->prefix!=children);
            stack.push_back(children);
            TrieNode* prefix = children->prefix;
            bool pattern_end = children->pattern_end;
            while(prefix && !pattern_end) {
                pattern_end = pattern_end || prefix->pattern_end;
                prefix = prefix->prefix;
            }
            children->pattern_end = pattern_end;
            children = children->sibling;
        }
    }
}

void fill_pattern_occurrence(TrieNode* suffix, const string& haystack, int pos, vector<pair<int, int>>& result) {
    for(int i=pos; i>=0 && suffix; --i) {
        char curr_char = haystack[i];
        suffix = list_search(suffix, curr_char);
        if(suffix->pattern_end) {
            // [i, pos+1) ~ [i, pos]
            result.push_back(make_pair(i, pos+1));
        }
        suffix = suffix->children;
    }
}

// <begin_pos, len>
vector<pair<int, int>> ac_search(TrieNode* root, TrieNode* suffix, const string& haystack) {
    assert(root!=nullptr);
    assert(suffix!=nullptr);

    vector<pair<int, int>> result;
    TrieNode* prev_matched = nullptr;
    TrieNode* curr_matched = nullptr;
    for(int i=0; i<haystack.size(); ++i) {
        char curr_char = haystack[i];
        while(prev_matched && !curr_matched) {
            curr_matched = list_search(prev_matched->children, curr_char);
            prev_matched = prev_matched->prefix;
        }
        if(curr_matched==nullptr)
            curr_matched = list_search(root, curr_char);
        if(curr_matched && curr_matched->pattern_end) {
            fill_pattern_occurrence(suffix, haystack, i, result);
        }
        prev_matched = curr_matched;
        curr_matched = nullptr;
    }
    return result;
}

void print_tree(TrieNode* root, bool newline=true) {
    while(root) {
        cout << "(" << root->this_char << "," << root->height << "," << root->pattern_end << ") ";
        print_tree(root->children, false);
        root = root->sibling;
    }
    if(newline) cout << endl;
}

int main(int argc, char** argv) {

    vector<string> patterns = {
        "announce",
        "annual",
        "annually",
    };
    string haystack = "annual_announce";

/*
    vector<string> patterns = {
        "abc",
        "abd",
        "bbc",
    };
    string haystack = "babcabcbbc";
*/

    TrieNode* prefix = build_trie(patterns);
    build_longest_trans_target(prefix);
    TrieNode* suffix = build_suffix(patterns);

    print_tree(prefix);
    print_tree(suffix);

    auto result = ac_search(prefix, suffix, haystack);

    for(auto& item : result) {
        cout << item.first << ", " << item.second << endl;
    }

    free_trie(prefix);
    free_trie(suffix);

    return 0;
}
