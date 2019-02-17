#include "Key.h"

using namespace std;
using namespace cv;

namespace PaperKeyboard {

    KeyType getKeyType(string str) {
        if (str == KEY_TYPE_BUTTON)
            return BUTTON;
        else if (str == KEY_TYPE_SLIDEBAR)
            return SLIDEBAR;
        return BUTTON;
    }

    Key::Key(Point x1_, Point x2_, Point y1_, Point y2_, KeyType type_, string dfVal) {
        x1 = x1_;
        x2 = x2_;
        y1 = y1_;
        y2 = y2_;
        type = type_;
        if (type == SLIDEBAR && dfVal == "")
            val = "0";
        else
            val = move(dfVal);
    }

    string Key::getVal(const Point p) {
        if (type == SLIDEBAR) {
            if (p.x < x1.x || p.x > x2.x || p.y < x1.y || p.y > y2.y) return "";

            float len = x2.x - x1.x;
            float dist = p.x - x1.x;
            int res = dist / len * 100;
            val = to_string(res);
        }
        return val;
    }

    string Key::serealize2str() const {
        string res = type == BUTTON ? KEY_TYPE_BUTTON : KEY_TYPE_SLIDEBAR;
        res += " ";
        if (type != SLIDEBAR) res += val + " ";
        res += to_string(x1.x) + " ";
        res += to_string(x1.y) + " ";
        res += to_string(x2.x) + " ";
        res += to_string(x2.y) + " ";
        res += to_string(y1.x) + " ";
        res += to_string(y1.y) + " ";
        res += to_string(y2.x) + " ";
        res += to_string(y2.y);
        return res;
    }


    void Key::draw(Mat &img, Scalar color) {
        line(img, x1, x2, color);
        line(img, y1, y2, color);
        line(img, x1, y1, color);
        line(img, x2, y2, color);
        if (type == BUTTON) {
            int baseline = 0;
            Size fSize = getTextSize(val, FONT_HERSHEY_COMPLEX, 1, 1, &baseline);
            Point textPos = Point(((x1.x + x2.x) - fSize.width) / 2,
                                  ((x1.y + y1.y) + fSize.height) / 2);
            putText(img, val, textPos, FONT_HERSHEY_COMPLEX, 1, color);
        } else if (type == SLIDEBAR) {
            line(img, Point(x1.x + 5, x1.y + ((y1.y - x1.y) / 2)),
                 Point(x2.x - 5, x2.y + ((y2.y - x2.y) / 2)), color, 2);
            int v = (x2.x - x1.x) * stoi(val) / 100;
            circle(img, Point(x1.x + 5 + v, x1.y + ((y1.y - x1.y) / 2)), 3, color, FILLED);
        }
    }

}