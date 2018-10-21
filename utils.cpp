#include "utils.h"

using namespace std;
using namespace cv;

double getDist(Point a, Point b) {
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}