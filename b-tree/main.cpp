#include "btree.h"
#include <iostream>
#include <time.h>
#include <cstdlib>
#include <set>
#include <chrono>
#include <random>

using namespace std;

int main(int argc, char** argv) {

 
    const size_t fill_n = 1000;
    size_t unique_n = 0;
    vector<size_t> fill_seq;

    cout << string(10, '-') << " START INSERT " << string(10, '-') << endl;

    srandom(time(NULL));
    size_t tree_root = b_tree_create();
    for(int i=fill_n; i>0; --i) {
        size_t key = (static_cast<size_t>(random())%1000000)+10;
        fill_seq.push_back(key);
        bool ret = b_tree_insert(&tree_root, key, key);
        if(false) {
            cout << "insert: " << key ;
            cout << endl;
            unique_n += ret;
            test_print_btree(tree_root);
        } else {
            cout << key << ", ";
            unique_n += ret;
        }
        check_btree_restrict(tree_root);
    }
    cout << endl;
    cout << "unique:" << unique_n << ", duplicate:" << fill_n - unique_n << endl;
    test_print_btree(tree_root);

    cout << string(10, '-') << " START QUERY " << string(10, '-') << endl;
    sort(fill_seq.begin(), fill_seq.end());
    size_t prev_key = 0;
    size_t match_count = 0;
    size_t unmatch_count = 0;
    for(auto key : fill_seq) {
        size_t value;
        bool ret = b_tree_search(tree_root, key, &value);
        assert(ret && key==value);
        match_count += 1;
        if(key-prev_key>2) {
            ret = b_tree_search(tree_root, key-1, &value);
            assert(ret==false);
            unmatch_count += 1;
        }
        prev_key = key;
    }
    cout << "match: " << match_count << " unmatch_count: " << unmatch_count << endl;

    cout << string(10, '-') << " START DELETE " << string(10, '-') << endl;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    shuffle(fill_seq.begin(), fill_seq.end(), std::default_random_engine(seed));
    set<size_t> deleted;
    size_t delete_exist = 0;
    size_t delete_unexist = 0;
    for(auto key : fill_seq) {
        size_t value;
        bool ret = b_tree_delete(&tree_root, key, &value);
        if(deleted.find(key)==deleted.end()) {
            delete_exist += 1;
            deleted.insert(key);
            assert(ret && key==value);
        } else {
            delete_unexist += 1;
            assert(!ret);
        }
        if(false) {
            cout << "delete: " << key ;
            cout << endl;
            test_print_btree(tree_root);
        } else {
            cout << key << ", ";
        }
        check_btree_restrict(tree_root);
    }
    cout << endl;
    cout << "exists: " << delete_exist << " unexists: " << delete_unexist << endl;
    test_print_btree(tree_root);
    return 0;
}
