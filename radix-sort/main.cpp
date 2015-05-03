#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <time.h>
#include <assert.h>
#include <iterator>

using namespace std;

int const GROUP_MASK_BIT = 4;
int const GROUP_BUCKET_SIZE = 0x1<<GROUP_MASK_BIT;
int const GROUP_MASK = (0x1<<GROUP_MASK_BIT)-1;
int const GROUP_SIZE_BIT = sizeof(size_t)*8;
int const GROUP_COUNT = GROUP_SIZE_BIT/GROUP_SIZE_BIT;


int counting_sort(size_t* input, size_t len, size_t* output, size_t shift) {
    assert(input!=output);

    size_t bucket[GROUP_BUCKET_SIZE] = {0};
    for(size_t idx=0; idx<len; ++idx) {
        bucket[(input[idx]>>shift) & GROUP_MASK] += 1;
    }
    for(size_t idx=1; idx<GROUP_BUCKET_SIZE; ++idx) {
        bucket[idx] += bucket[idx-1];
    }
    // stable
    for(size_t idx=len; idx>0;) {
        --idx;
        size_t target_pos = --bucket[(input[idx]>>shift) & GROUP_MASK];
        output[target_pos] = input[idx];
    }
    return 0;
}

void radix_sort(size_t* nums, size_t len) {
    if(len<=0) return;

    size_t* buffer = new size_t[len];
    size_t* input = nums;
    size_t* output = buffer;

    for(int shift=0; shift<GROUP_SIZE_BIT; shift+=GROUP_MASK_BIT) {
        counting_sort(input, len, output, shift);
        size_t* temp = input;
        input = output;
        output = temp;
    }

    if(output!=nums) {
        copy(output, output+len, nums);
    }

    delete[] buffer;
}


int main(int argc, char** argv) {

    const size_t test_size= 10240000;
    size_t* test = new size_t[test_size];
    size_t* origin = new size_t[test_size];
    srandom(time(NULL));
    for(size_t i=0; i<test_size; ++i) {
        origin[i] = test[i] = static_cast<size_t>(random());
    }

    radix_sort(test, test_size);
    sort(origin, origin+test_size);

    for(size_t i=0; false && i<test_size; ++i) {
        assert(test[i]==origin[i]);
    }

    return 0;
}





