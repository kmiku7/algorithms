#include <string>
#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <vector>

using namespace std;

class SuffixTree{
    const char* str;
    size_t len;
    struct TreeNode{
        // [begin, end]
        size_t begin;
        size_t end;
        TreeNode* next_sib;
        TreeNode* child;
        TreeNode() : begin(0), end(0), next_sib(nullptr), child(nullptr) {}
    }*root;
public:
    SuffixTree() : str(nullptr), len(0), root(nullptr){}
    bool init(const char* origin_str) {
        size_t origin_len = strlen(origin_str);
        str = static_cast<const char*>(malloc(origin_len+1));
        memcpy(const_cast<char*>(str), origin_str, origin_len+1);
        len = origin_len + 1;

        // build tree
        root = new TreeNode();
        root->begin = 0;
        root->end = len-1;
        for(int begin_pos = 1; begin_pos < len; ++begin_pos) {
            size_t sub_begin_pos = begin_pos;
            TreeNode* parent = nullptr;
            TreeNode* prev_node = nullptr;
            TreeNode* curr_node = root;
            while(sub_begin_pos<len) {
                // find insert pos
                while(curr_node && str[curr_node->begin]<str[sub_begin_pos]) {
                    prev_node = curr_node;
                    curr_node = curr_node->next_sib;
                }
                if(curr_node==nullptr || str[curr_node->begin]>str[sub_begin_pos]) {
                    TreeNode* temp = new TreeNode();
                    temp->begin = sub_begin_pos;
                    temp->end = len-1;
                    temp->next_sib = curr_node;
                    if(prev_node) prev_node->next_sib = temp;
                    else if(parent) parent->child = temp;
                    else root = temp;
                    sub_begin_pos = len;
                } else {
                    // split node, find longest prefix
                    size_t sub_curr_pos = sub_begin_pos+1;
                    size_t node_curr_pos = curr_node->begin+1;
                    while(sub_curr_pos<len && node_curr_pos<=curr_node->end && str[sub_curr_pos]==str[node_curr_pos]) {
                        ++sub_curr_pos;
                        ++node_curr_pos;
                    }
                    if(sub_curr_pos==len && node_curr_pos>curr_node->end) {
                        // the suffix is shorter
                        assert(false);
                    } else if(sub_curr_pos==len) {
                        // the suffix is shorter and end with '\0'
                        assert(false);
                    } else if(node_curr_pos>curr_node->end) {
                        parent = curr_node;
                        prev_node = nullptr;
                        curr_node = curr_node->child;
                        sub_begin_pos = sub_curr_pos;
                    } else {
                        // complicated
                        TreeNode* first_half = new TreeNode();
                        first_half->begin = curr_node->begin;
                        first_half->end = node_curr_pos-1;
                        first_half->next_sib = curr_node->next_sib;
                        first_half->child = curr_node;
                        curr_node->begin = node_curr_pos;
                        curr_node->next_sib = nullptr;
                        TreeNode* sub_second_half = new TreeNode();
                        sub_second_half->begin = sub_curr_pos;
                        sub_second_half->end = len-1;
                        TreeNode* first_child;
                        if(str[sub_curr_pos]<str[node_curr_pos]) {
                            sub_second_half->next_sib = curr_node;
                            first_child = sub_second_half;
                        } else {
                            curr_node->next_sib = sub_second_half;
                            first_child = curr_node;
                        }
                        first_half->child = first_child;
                        if(prev_node) prev_node->next_sib = first_half;
                        else if(parent) parent->child = first_half;
                        else root = first_half;
                        sub_begin_pos = len;
                    }
                }
            }
        }
    }
    bool is_sub(const char* pattern) {
        return first_occurence_pos(pattern, nullptr);
    }
    bool first_occurence_pos(const char* pattern, size_t* pos) {
        TreeNode* curr_node = root;
        const char* pattern_curr_pos = pattern;
        while(curr_node) {
            while(curr_node && str[curr_node->begin]<*pattern_curr_pos) curr_node = curr_node->next_sib;
            if(!curr_node || str[curr_node->begin]>*pattern_curr_pos) return false;
            size_t suffix_pos = curr_node->begin;
            while(*pattern_curr_pos && suffix_pos <=curr_node->end && *pattern_curr_pos==str[suffix_pos]) {
                ++suffix_pos;
                ++pattern_curr_pos;
            }
            if(*pattern_curr_pos=='\0'){
                if(pos) *pos = suffix_pos - (pattern_curr_pos - pattern);
                return true;
            }
            else if(suffix_pos<=curr_node->end) return false;
            else {
                curr_node = curr_node->child;
            }
        }
        return false;
    }
    void print_tree(TreeNode* root, int level=0) {
        if(!root) return;
        if(!level) cout << string(10, '-') << endl;
        while(root) {
            printf("(level:%d,begin:%ld,end:%ld) ", level, root->begin, root->end);
            print_tree(root->child, level+1);
            root = root->next_sib;
        }
        if(!level) cout << endl << string(20, '-') << endl << flush;
    }
    int get_static() {
        vector<TreeNode*> list;
        list.push_back(root);
        size_t node_count = 0;
        while(!list.empty()) {
            auto node = list.back();
            list.pop_back();
            if(!node) continue;
            node_count += 1;
            list.push_back(node->next_sib);
            list.push_back(node->child);
        }
        return node_count;
    }
};



int main(int argc, char** argv) {

    SuffixTree tree_a;
    //string aa = "mississippi";
    //string bb = "issi";
    string aa = "aaaaaa";
    string bb = "issi";
    size_t pos_a = 99;
    tree_a.init(aa.c_str());
    cout << tree_a.is_sub(bb.c_str()) << endl;
    tree_a.first_occurence_pos(bb.c_str(), &pos_a);
    cout << pos_a << endl;
    cout << aa.size() << ", " << tree_a.get_static() << endl;


    string paper = "As is the case in radio frequency transmission systems, multipath propagation \
effects are important for wireless optical networks. The power launched from \
the transmitter may take many reflected and refracted paths before arriving at \
the receiver. In radio systems, the sum of the transmitted signal and its images \
at the receive antenna cause spectral nulls in the transmission characteristic. \
These nulls are located at frequencies where the phase shift between the paths \
causes destructive interference at the receiver. This effect is known as multipath \
fading [19]. \
Unlike radio systems, multipath fading is not a major impairment in wireless \
optical transmission. The “antenna” in a wireless optical system is the light \
detector which typically has an active radiation collection area of approximately \
The relative size of this antenna with respect to the wavelength of the \
infrared light is immense, on the order of The multipath propagation of \
light produces fades in the amplitude of the received electromagnetic signal at \
spacings on the order of half a wavelength apart. As mentioned earlier, the light \
detector is a square law device which integrates the square of the amplitude of \
the electromagnetic radiation impinging on it. The large size of the detector \
with respect to the wavelength of the light provides a degree of inherent spatial \
diversity in the receiver which mitigates the impact of multipath fading [2]. \
Although multipath fading is not a major impediment to wireless optical \
links, temporal dispersion of the received signal due to multipath propagation \
remains a problem. This dispersion is often modelled as a linear time invariant \
system since the channel properties change slowly over many symbol periods \
[ 1, 20]. The impact of multipath dispersion is most noticeable in diffuse infrared \
communication systems, which are described in more detail in Section 2.4.2. In \
short distance line-of-sight (LOS) links, presented in Section 2.4.1, multipath \
dispersion is seldom an issue. Indeed, channel models proposed for LOS links \
assume the LOS path dominates and model the channel as a linear attenuation \
and delay [21].";
    string target = "dispersion";
    SuffixTree tree_b;
    size_t pos_b = 99;
    tree_b.init(paper.c_str());
    tree_b.first_occurence_pos(target.c_str(), &pos_b);
    cout << tree_b.is_sub(target.c_str()) << endl;
    cout << pos_b << endl;
    cout << paper.size() << ", " << tree_b.get_static() << endl;
    return 0;
}
