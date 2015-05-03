#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

int my_lower_bound(int* nums, size_t len, int target) {
    if(len<=0 || target<=nums[0])
        return 0;
    else if(target>nums[len-1])
        return len;
    else {
        size_t begin = 0;
        size_t end = len - 1;
        while(end-begin>1) {
            size_t mid = begin+(end-begin)/2;
            if(target<=nums[mid])
                end = mid;
            else
                begin = mid;
        }
        return end;
    }
}

int my_upper_bound(int* nums, size_t len, int target) {
    if(len<=0 || target<nums[0])
        return 0;
    else if(target>=nums[len-1])
        return len;
    else {
        size_t begin = 0;
        size_t end = len - 1;
        while(end-begin>1) {
            size_t mid = begin+(end-begin)/2;
            if(target<nums[mid])
                end = mid;
            else
                begin = mid;
        }
        return end;
    }
}

int main(int argc, char** argv) {

    size_t  len = 100;
    int     nums[100];
    for(int i=1; i<=100; ++i) {
        nums[i-1]=i;
    }

    for(int i=0; i<=101; ++i) {
        int r1 = my_lower_bound(nums, len, i);
        int r2 = lower_bound(nums, nums+len, i) - nums;
        //cout << i << ", " << r1 << ", " << r2 << endl;
        assert(r1==r2);
    }

    cout << "-----" << endl;

    for(int i=0; i<=101; ++i) {
        int r1 = my_upper_bound(nums, len, i);
        int r2 = upper_bound(nums, nums+len, i) - nums;
        //cout << i << ", " << r1 << ", " << r2 << endl;
        assert(r1==r2);
    }

    return 0;
}
