#ifndef PAPERKEYBOARD_PAPERKEYBOARD_H
#define PAPERKEYBOARD_PAPERKEYBOARD_H

#include <opencv2/opencv.hpp>

#include <future>

#include "Key.h"

#include "handDetector/utils.h"
#include "handDetector/HandDetector.h"

#include "QR.h"
#include "utils.h"

#include <fstream>

using namespace std;
using namespace cv;

namespace PaperKeyboard {

#define A4_SIZE Size(595, 842)
#define GaugeLineLength 100 // px
#define GL_KB_INDENT 10 // indent between gauge line and keyboard
#define PKB_HEADER "PKB"
#define PKB_STR_TYPE_CHANGE '%'
#define DATA_SEPARATOR '\n'
#define CMD_START_SYM "$"
#define ONCLICK_CALLBACK function<void(const Point &, Key &)>

#define COLOR_WHITE Scalar(255, 255, 255)
#define COLOR_BLACK Scalar(0, 0, 0)


    enum QRCodePos {
        NONE = 0,
        TOP_LEFT = 1,
        BTM_RIGHT = 2
    };

    struct PrintType {
        PrintType(bool gaugeLine, QRCodePos qrCOdePos);

        explicit PrintType(string str);

        bool gaugeLine;
        QRCodePos qrCOdePos;

        string serialize();
    };


    enum Click_rec_finger { // finger from which recognizing pressing
        FARTHEST = 0,
        HIGHER = 1,
        ALL = 2
    };

    class PaperKeyboard {
    public:
        HandDetector hd;
        Scalar YCrCb_lower = Scalar(0, 135, 90);
        Scalar YCrCb_upper = Scalar(255, 230, 150);

        vector<Key> keys;
        vector<vector<Point>> keysPositions;
        Size fontSize = Size(10, 10);

        ONCLICK_CALLBACK onClick;

        time_t lastClickTime = time(nullptr);
        float clickDelay = 1; // seconds
        int minDistChange = 20;
        int maxDistChange = 120;

        Click_rec_finger clrf = ALL;

        Ptr<BackgroundSubtractorMOG2> bgs;
        bool bgs_learn = true;
        int bgs_learnNFrames = 100;

        vector<Point> scaleLine;
        vector<string> keysVec;

        PrintType printType = PrintType(false, QRCodePos::NONE);

        PaperKeyboard();

        Mat detectHands(Mat img);

        void updateBG();

        void addKey(Point x1, Point x2, Point y1, Point y2, KeyType type, string text = "");

        vector<vector<Point>> getKeysPositions(Point x1, Size ksize = Size(50, 50));

        void addKeysByVec(Point x1, Size ksize = Size(50, 50));

        void adjustKeyboardByQR(Mat img);

        void prepare4Print(Mat &img);

        string serializeKeys2str();
        string serialize2str();

        bool praseString(string str, Point startPoint = Point(0, 0));

        bool loadFromFile(string filePath, Point startPoint = Point(0, 0));

        bool save2file(string filePath);

        // set num as -1 to not use limitation
        void deleteKeysByText(string text, int num = 1);

        Key *getKeyByPoint(Point p);

        void setOnclick(ONCLICK_CALLBACK f);

        void callOnclick(const Point &p, Key &k, bool runAsync = false);

        void getClicks();

        void drawKeys(Mat &img, Scalar color = Scalar(255, 0, 0));

        void draw(Mat &img, Scalar color = Scalar(255, 0, 0));

    private:
        bool onClickSet = false;
    };

}
#endif //PAPERKEYBOARD_PAPERKEYBOARD_H