#ifndef PKBUTILS_HPP
#define PKBUTILS_HPP

#include <cmath>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

double getDist(Point a, Point b)
{
   return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

#endif // !PKBUTILS_HPP