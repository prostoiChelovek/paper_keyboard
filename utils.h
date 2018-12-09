#ifndef PAPERKEYBOARD_UTILS_H
#define PAPERKEYBOARD_UTILS_H

#include "string"
#include "vector"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

size_t split(const string &txt, vector<string> &strs, char ch);

vector<int> stoiAll(vector<string> v);

double getAngle(Point p1, Point p2);

void rotateImg(Mat &img, double angle);

#endif //PAPERKEYBOARD_UTILS_H
