#include "utils.h"

size_t split(const string &txt, vector<string> &strs, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();
    while (pos != string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }
    strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));
    return strs.size();
}

vector<int> stoiAll(vector<string> v) {
    vector<int> res;
    for (const string &s : v) {
        try {
            res.emplace_back(stoi(s));
        } catch (...) {
            continue;
        }
    }
    return res;
}

double getAngle(Point p1, Point p2) {
    return atan2(p1.y - p2.y, p1.x - p2.x) * 180 / CV_PI;
}

void rotateImg(Mat &img, double angle) {
    Point2f pt(img.cols / 2., img.rows / 2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine(img, img, r, Size(img.cols, img.rows));
}

ShortFinger getSame(const vector<ShortFinger> &fingers, const Finger &f) {
    int minDist;
    ShortFinger res;
    for (const ShortFinger &fn : fingers) {
        int crDist = getDist(fn.ptStart, f.ptStart) +
                     getDist(fn.ptEnd, f.ptEnd) +
                     getDist(fn.ptFar, f.ptFar);
        if (crDist < minDist) {
            minDist = crDist;
            res = fn;
        }
    }
    return res;
}