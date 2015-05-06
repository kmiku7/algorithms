#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

typedef pair<int, int> Point;
typedef pair<long long, long long> SquareCosTheta;

void print_point(const Point& point) {
    cout << "(" << point.first << ", " << point.second << ")";
}

void print_points(const vector<Point>& points) {
    for(auto& p : points)
        print_point(p);
}

unsigned gcd(unsigned x, unsigned y) {
    if(y==0) return x;
    unsigned rem = x % y;
    while(rem) {
        x = y;
        y = rem;
        rem = x % y;
    }
    return y;
}

SquareCosTheta square_of_cos_theta(const Point& p0, const Point& p1, const int direction_x) {
    assert(direction_x!=0);
    assert(p0!=p1);

    long long v0_x = direction_x > 0 ? 1 : -1;
    long long v0_y = 0;
    long long v1_x = p1.first - p0.first;
    long long v1_y = p1.second - p0.second;
    long long inner_product = (v0_x * v1_x) + (v0_y * v1_y);
    long long square_v1_len = v1_x*v1_x + v1_y*v1_y;
    int sign = inner_product >=0 ? 1 : -1;
    inner_product = inner_product * inner_product;
    long long shrink = gcd(inner_product, square_v1_len);

    return make_pair(sign*inner_product/shrink, square_v1_len/shrink);
}

bool great_to(const SquareCosTheta& left, const SquareCosTheta& right) {
    // 无符号整数剪发注意下溢。
    cout << left.first << "," << left.second << "\t" << right.first << "," << right.second << endl;
    return (left.first*right.second - left.second*right.first) > 0;
}

vector<Point> get_convex_hull(const vector<Point>& points) {
    if(points.size()<=1) return points;

    vector<Point> result = points;
    int max_pos = 0;
    int convex_point_count = 0;
    bool find = false;
    // find p0
    for(int i=1; i<result.size(); ++i) {
        if(result[i].second < result[max_pos].second 
            || (result[i].second == result[max_pos].second && result[i].first < result[max_pos].first))
            max_pos = i;
    }
    convex_point_count += 1;
    if(max_pos!=0)
        swap(result[0], result[max_pos]);
    // Elegant! I think.
    result.push_back(result[0]);


    vector<int> directions = {1, -1};
    for(auto direction : directions) {
        do {
            Point& prev_base = result[convex_point_count-1];
            max_pos = -1;
            SquareCosTheta max_value;
            for(int i=convex_point_count; i<result.size(); ++i) {
                if((result[i].second-prev_base.second)*direction<0 || result[i]==prev_base) continue;
                SquareCosTheta curr_value = square_of_cos_theta(prev_base, result[i], direction);
                if(max_pos==-1 
                    || great_to(curr_value, max_value)
                    || (curr_value == max_value 
                        && ((result[i].first-result[max_pos].first)*direction>=0 
                            || result[i].second-result[max_pos].second)*direction>=0)) {
                    max_pos = i;
                    max_value = curr_value;
                }
            }
            if(max_pos==-1)
                break;
            swap(result[convex_point_count], result[max_pos]);
            convex_point_count += 1;
        }while(true);
    }

    result.resize(convex_point_count-1);
    result.shrink_to_fit();

    return result;
}

bool read_points(const char* filename, vector<Point>& ret) {
    ifstream fin(filename);
    if(!fin) {
        fprintf(stderr, "Invalid file:%s\n", filename);
        return false;
    }

    int count = 0;
    fin >> count;
    int remain = count;
    int x;
    int y;
    fin >> x >> y;
    while(fin && remain > 0) {
        remain -= 1;
        ret.push_back(make_pair(x, y));
        fin >> x >> y;
    }
    return ret.size()==count;
}

int main(int argc, char** argv) {
    if(argc!=2) {
        fprintf(stderr, "%s test-file\n", argv[0]);
        return 1;
    }
    vector<Point> points;

    if(!read_points(argv[1], points))
        return 2;;

    print_points(points);
    cout << endl;
    vector<Point> ret = get_convex_hull(points);
    print_points(ret);
    cout << endl;
    return 0;
}
