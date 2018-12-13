#ifndef PAPERKEYBOARD_PKBKEY_H
#define PAPERKEYBOARD_PKBKEY_H

#include <opencv2/opencv.hpp>

#include "utils.h"

using namespace std;
using namespace cv;

enum pkb_key_type {
    BUTTON,
    SLIDEBAR
};

#define PKBK_TYPE_BUTTON "0"
#define PKBK_TYPE_SLIDEBAR "1"

pkb_key_type getPKBKType(string str);

class PKBKey {
public:
    Point x1 = Point(-1, -1);
    Point x2, y1, y2;
    string text;
    pkb_key_type type = BUTTON;

    int val = 0;

    PKBKey() = default;

    PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, string text_);

    PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, pkb_key_type kType_, string text_ = "");

    float getSliderVal(Point p);

    string getVal(Point p = Point(0, 0));

    string serealize2str() const;

    void draw(Mat &img, Scalar color = Scalar(255, 0, 0),
              Size fontSize = Size(10, 10));
};

#endif //PAPERKEYBOARD_PKBKEY_H
