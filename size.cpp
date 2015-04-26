#include <iostream>

using namespace std;

#define SIZEOF(type) cout << #type << ":\t" << sizeof(type) << endl;

int main(int argc, char** argv) {

    SIZEOF(int);
    SIZEOF(long);
    SIZEOF(long long);
    SIZEOF(size_t);
    SIZEOF(float);
    SIZEOF(double);
    SIZEOF(void*);

    return 0;
    
}
