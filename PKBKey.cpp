#include "PKBKey.h"

using namespace std;
using namespace cv;

pkb_key_type getPKBKType(string str) {
    if (str == PKBK_TYPE_BUTTON)
        return BUTTON;
    else if (str == PKBK_TYPE_SLIDEBAR)
        return SLIDEBAR;
    return BUTTON;
}

PKBKey::PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, string text_) {
    x1 = x1_;
    x2 = x2_;
    y1 = y1_;
    y2 = y2_;
    text = move(text_);
}

PKBKey::PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, pkb_key_type type_, string text_) {
    x1 = x1_;
    x2 = x2_;
    y1 = y1_;
    y2 = y2_;
    text = move(text_);
    type = type_;
}

float PKBKey::getSliderVal(const Point p) {
    if (type != SLIDEBAR) return -1;
    if (p.x < x1.x || p.x > x2.x || p.y < x1.y || p.y > y2.y) return -1;

    float len = x2.x - x1.x;
    float dist = p.x - x1.x;
    float res = dist / len * 100;
    val = res;
    return res;
}

string PKBKey::getVal(const Point p) {
    if (type == SLIDEBAR)
        return to_string(getSliderVal(p));
    else if (type == BUTTON)
        return text;
    return "";
}

string PKBKey::serealize2str() const {
    string res = type == BUTTON ? PKBK_TYPE_BUTTON : PKBK_TYPE_SLIDEBAR;
    res += " ";
    if (type != SLIDEBAR) res += text + " ";
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


void PKBKey::draw(Mat &img, Scalar color, Size fontSize) {
    line(img, x1, x2, color);
    line(img, y1, y2, color);
    line(img, x1, y1, color);
    line(img, x2, y2, color);
    if (type == BUTTON) {
        putText(img, text,
                Point((x1.x + x2.x) / 2 - fontSize.width * (text.size()),
                      (x1.y + y1.y) / 2 + fontSize.height),
                FONT_HERSHEY_COMPLEX, 1, color);
    } else if (type == SLIDEBAR) {
        line(img, Point(x1.x + 5, x1.y + ((y1.y - x1.y) / 2)),
             Point(x2.x - 5, x2.y + ((y2.y - x2.y) / 2)), color, 2);
        int v = (x2.x - x1.x) * val / 100;
        circle(img, Point(x1.x + 5 + v, x1.y + ((y1.y - x1.y) / 2)), 3, color, FILLED);
    }
}
