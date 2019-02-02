#ifndef PAPERKEYBOARD_PKBKEY_H
#define PAPERKEYBOARD_PKBKEY_H

#include <opencv2/opencv.hpp>

#include "utils.h"

using namespace std;
using namespace cv;

namespace PaperKeyboard {

    enum KeyType {
        BUTTON,
        SLIDEBAR
    };

#define KEY_TYPE_BUTTON "0"
#define KEY_TYPE_SLIDEBAR "1"

    KeyType getKeyType(string str);

    class Key {
    public:
        Point x1 = Point(-1, -1);
        Point x2, y1, y2;
        string text;
        KeyType type = BUTTON;

        int val = 0;

        Key() = default;

        Key(Point x1_, Point x2_, Point y1_, Point y2_, string text_);

        Key(Point x1_, Point x2_, Point y1_, Point y2_, KeyType kType_, string text_ = "");

        float getSliderVal(Point p);

        string getVal(Point p = Point(0, 0));

        string serealize2str() const;

        void draw(Mat &img, Scalar color = Scalar(255, 0, 0),
                  Size fontSize = Size(10, 10));
    };

}

#endif //PAPERKEYBOARD_PKBKEY_H
