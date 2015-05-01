#include <string>
#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <vector>
#include "ukkonens.h"

using namespace std;
using namespace suffix_tree_ukkonens;


void test_case(const string& str, const string& pattern) {
    SuffixTree<TreeNodeList> tree;
    cout << str << endl;
    cout << pattern << endl;
    tree.append_str(str.c_str());
    //tree.print_tree();
    cout << tree.search_pattern(pattern.c_str()) << endl;
    return;
    for(int i=0; i<str.size(); ++i) {
        //cout << "find:" << str.c_str() + i << ", idx:" << i << ", ";
        cout << tree.search_pattern(str.c_str()+i) << endl;
    }
}

int main(int argc, char** argv) {

    SuffixTree<TreeNodeList> tree_a;
    string a1 = "mississippi";
    string a2 = "abcabxabcd";
    string a3 = "abcabxabcdbcabyabcabxy";
    string a4 = "1112121121112";
    string bb = "ippi";

    test_case(a4, bb);
    test_case(a2, bb);
    test_case(a1, bb);

    string paper = "As is the case in radio frequency transmission systems, multipath propagation \
effects are important for wireless optical networks. The power launched from \
the transmitter may take many reflected and refracted paths before arriving at \
the receiver. In radio systems, the sum of the transmitted signal and its images \
at the receive antenna cause spectral nulls in the transmission characteristic. \
These nulls are located at frequencies where the phase shift between the paths \
causes destructive interference at the receiver. This effect is known as multipath \
fading [19]. \
Unlike radio systems, multipath fading is not a major impairment in wireless \
optical transmission. The \"antenna\" in a wireless optical system is the light \
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
    //string target = "dispersion";
    string target = "are";
    string sentence = "cause spectral nulls in the transmission";
    test_case(sentence, target);
    test_case(paper, target);

    return 0;
}
