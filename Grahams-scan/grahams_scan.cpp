#include <vector>
#include <algorithm>
#include <climits>
#include <iostream>
#include <fstream>

using namespace std;

typedef pair<int, int> Point;

void print_points(const vector<Point>& points) {
    for(auto p : points) 
        cout << "(" << p.first << ", " << p.second << ") ";
    cout << endl;
}

int cross_product(const Point& p0, const Point& p1, const Point& p2) {
    return (p1.first-p0.first)*(p2.second-p0.second) - (p1.second-p0.second)*(p2.first-p0.first);
}

void sort_and_filter_point(vector<Point>& points, const Point& p0) {
    vector<Point> result;
    result.push_back(points[0]);
    for(int i=1; i<points.size(); ++i) {
        int begin = 0;
        int end = result.size();
        int cp = 0;
        while(begin<end) {
            int mid = begin + (end-begin)/2;
            cp = cross_product(p0, points[i], result[mid]);
            if(cp<0) begin = mid + 1;
            else end = mid;
            // fatal [begin, end)
            // else end = mid - 1;
        }

        if(begin>=result.size() || 0!=(cp=cross_product(p0, points[i], result[begin]))) {
            Point tmp = points[i];
            size_t origin_size = result.size();
            result.resize(result.size()+1);
            // 注意这个函数参数的含义，尤其是最后一个last
            copy_backward(result.begin()+begin, result.begin()+origin_size, result.end());
            result[begin] = tmp;
        } else if((abs(points[i].first-p0.first) > abs(result[begin].first-p0.first)) 
                    || (abs(points[i].second-p0.second) > abs(result[begin].second-p0.second)))  {
            result[begin] = points[i];
        }
    }
    points.swap(result);
}

bool is_turn_left(const Point& next_to_top, const Point& top, const Point& point) {
    return 0 < cross_product(next_to_top, top, point);
}

vector<Point> get_convex_hull(vector<Point>& points) {
    if(points.size()<=1) return points;
    Point p0 = make_pair(INT_MAX, INT_MAX);
    for(auto p : points) {
        if(p.second<p0.second || (p.second==p0.second && p.first<p0.first)) {
            p0 = p;
        }
    }

    vector<Point> candidates;;
    candidates.reserve(points.size());
    for(auto p : points) {
        if(p!=p0) candidates.push_back(p);
    }
    sort_and_filter_point(candidates, p0);
    vector<Point> result;
    result.push_back(p0);
    result.push_back(candidates[0]);
    if(points.size()==2)
        return result;

    result.push_back(candidates[1]);

    for(int i=2; i<candidates.size(); ++i) {
        //Point& top = result.back();
        //Point& next_to_top = result[result.size()-2];
        if(!is_turn_left(result[result.size()-2], result.back(), candidates[i]))
            result.pop_back();
        result.push_back(candidates[i]);
    }
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
    while(fin && remain>0) {
        remain -= 1;
        ret.push_back(make_pair(x,y));
        fin >> x >> y;
    }
    return ret.size()==count;
}
int main(int argc, char** argv) {

    // test-file
    // line 1: points-nums
    // line 2: x1 y1
    // ...
    if(argc!=2) {
        fprintf(stderr, "%s test-file\n", argv[0]);
        return 1;
    }
    vector<Point> points;

    if(!read_points(argv[1], points))
        return 2;

    print_points(points);
    vector<Point> ret = get_convex_hull(points);
    print_points(ret);

    return 0;
}
