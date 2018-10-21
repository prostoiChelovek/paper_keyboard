#ifndef PAPERKEYBOARD_PKBKEY_H
#define PAPERKEYBOARD_PKBKEY_H

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class PKBKey {
public:
    Point x1 = Point(-1, -1);
    Point x2, y1, y2;
    string text;

    PKBKey() = default;

    PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, string text_);

    void draw(Mat &img, Scalar color = Scalar(255, 0, 0),
              Size fontSize = Size(10, 10));
};

#endif //PAPERKEYBOARD_PKBKEY_H
