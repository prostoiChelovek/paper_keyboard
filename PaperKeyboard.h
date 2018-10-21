#ifndef PAPERKEYBOARD_PAPERKEYBOARD_H
#define PAPERKEYBOARD_PAPERKEYBOARD_H

#include <opencv2/opencv.hpp>

#include <future>

#include "PKBKey.h"
#include "utils.h"

#include "handDetector/HandDetector.h"

class PaperKeyboard {
public:
    Mat bg;

    HandDetector hd;
    Scalar YCrCb_lower = Scalar(0, 135, 90);
    Scalar YCrCb_upper = Scalar(255, 230, 150);

    vector<PKBKey> keys;
    Size fontSize = Size(10, 10);

    function<void(const Point &, const PKBKey &)> onClick;
    bool onClickSet = false;

    int lastDist;
    time_t lastClickTime = time(0);
    int minDistChange = -5;
    int maxDistChange = 10;
    int clickDelay = 1;

    vector<string> keysVec;
    String adjRngWName = "adjust color ranges";
    String adjKbWName = "adjust keyboard";

    PaperKeyboard();

    Mat detectHands(Mat img);

    // TODO: fix this
    void setLast();

    void adjustColorRanges();

    void adjustKeyboard(Mat &img);

    void addKey(Point x1, Point x2, Point y1, Point y2, string text);

    void addKeysByVec(Point x1, Size ksize = Size(50, 50));

    void deleteKeysByText(string text, int num = 1);

    PKBKey getKeyByPoint(Point p);

    void setOnclick(function<void(const Point &, const PKBKey &)> f);

    void callOnclick(const Point &p, const PKBKey &k, bool runAsync = true);

    void getClicks();

    void drawKeys(Mat &img, Scalar color = Scalar(255, 0, 0));

    void draw(Mat &img, Scalar color = Scalar(255, 0, 0));

private:
    vector<Point> tmpPoints;
};


#endif //PAPERKEYBOARD_PAPERKEYBOARD_H
