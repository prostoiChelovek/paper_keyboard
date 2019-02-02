#ifndef PAPERKEYBOARD_PAPERKEYBOARD_H
#define PAPERKEYBOARD_PAPERKEYBOARD_H

#include <opencv2/opencv.hpp>

#include <future>

#include "PKBKey.h"

#include "handDetector/utils.h"
#include "handDetector/HandDetector.h"

#include "QR.h"
#include "utils.h"

#include <fstream>

using namespace std;
using namespace cv;

#define A4_SIZE Size(595, 842)
#define GaugeLineLength 100 // px

#define GL_KB_INDENT 10 // indent between gauge line and keyboard

#define COLOR_WHITE Scalar(255, 255, 255)
#define COLOR_BLACK Scalar(0, 0, 0)

#define PKB_PRINT_TYPE_1 1 // without qr code and gauge line
#define PKB_PRINT_TYPE_2 2 // without qr code and with gauge line
#define PKB_PRINT_TYPE_3 3 // without gauge line
#define PKB_PRINT_TYPE_4 4 // qr code on a separate sheet
#define PKB_PRINT_TYPES_NUM 4

#define PKB_HEADER "%PKB%"

enum Click_rec_finger { // finger from which recognizing pressing
    FARTHEST,
    HIGHER,
    ALL
};

class PaperKeyboard {
public:
    Mat bg;

    HandDetector hd;
    Scalar YCrCb_lower = Scalar(0, 135, 90);
    Scalar YCrCb_upper = Scalar(255, 230, 150);

    vector<PKBKey> keys;
    vector<vector<Point>> keysPositions;
    Size fontSize = Size(10, 10);

    function<void(const Point &, PKBKey &)> onClick;
    bool onClickSet = false;

    time_t lastClickTime = time(nullptr);
    float clickDelay = 1; // seconds
    int minDistChange = 20;
    int maxDistChange = 120;

    Click_rec_finger clrf = ALL;

    vector<Point> scaleLine;
    vector<string> keysVec;

    String adjRngWName = "adjust color ranges";
    String adjKbWName = "adjust keyboard";
    String adjScWName = "adjust scale";

    PaperKeyboard();

    Mat detectHands(Mat img);

    void setLast();

    void adjustColorRanges();

    void adjustKeyboardManually(Mat &img);

    void adjustScale(Mat &img); // do not call together with adjustKeyboardManually

    void addKey(Point x1, Point x2, Point y1, Point y2, string text);

    void addKey(Point x1, Point x2, Point y1, Point y2, pkb_key_type type, string text = "");

    vector<vector<Point>> getKeysPositions(Point x1, Size ksize = Size(50, 50));
    void addKeysByVec(Point x1, Size ksize = Size(50, 50));

    void adjustKeyboardByQR(Mat img);

    void prepare4Print(Mat &img, int printType = 1);

    string serialize2str(int printType = 1);

    // if mode == PKB_PRINT_TYPE_4 - using startPoint
    // else - using scaleLine
    bool deserializeFromString(string str, Point startPoint = Point(0, 0));

    bool loadFromFile(string filePath, Point startPoint = Point(0, 0));

    bool save2file(string filePath, int printType = 1);

    // set num as -1 to not use limitation
    void deleteKeysByText(string text, int num = 1);

    PKBKey getKeyByPoint(Point p);

    void setOnclick(function<void(const Point &, PKBKey &)> f);

    void callOnclick(const Point &p, PKBKey &k, bool runAsync = false);

    void getClicks();

    void drawKeys(Mat &img, Scalar color = Scalar(255, 0, 0));

    void draw(Mat &img, Scalar color = Scalar(255, 0, 0));

    void clearTmpPoints();
private:
    vector<Point> tmpPoints;
    vector<vector<ShortFinger>> lastFingers;
};

#endif //PAPERKEYBOARD_PAPERKEYBOARD_H