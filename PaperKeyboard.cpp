#include "PaperKeyboard.h"

using namespace std;
using namespace cv;

PaperKeyboard::PaperKeyboard() {
    hd = HandDetector();
}

Mat PaperKeyboard::detectHands(Mat img) {
    Mat imgYCrCb, mask;
    hd.deleteBg(img, bg, img);

    cvtColor(img, imgYCrCb, COLOR_BGR2YCrCb);
    mask = hd.detectHands_range(imgYCrCb, YCrCb_lower, YCrCb_upper);

    hd.getFingers();
    hd.getCenters();
    hd.getHigherFingers();
    hd.getFarthestFingers();

    return mask;
}

// TODO: fix this
void PaperKeyboard::setLast() {
    if (!hd.hands.empty()) {
        if (hd.hands[0].farthestFinger.ok)
            lastDist = getDist(hd.hands[0].farthestFinger.ptFar, hd.hands[0].farthestFinger.ptStart);
    }
}

void PaperKeyboard::adjustColorRanges() {
    namedWindow(adjRngWName);
    auto onTrackbarActivity = [](int val, void *data) {
        double &vNum = *(static_cast<double *>(data));
        vNum = double(val);
    };
    createTrackbar("CrMin", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[0]);
    createTrackbar("CrMax", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[0]);
    createTrackbar("CbMin", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[1]);
    createTrackbar("CbMax", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[1]);
    createTrackbar("YMin", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[2]);
    createTrackbar("YMax", adjRngWName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[2]);
    setTrackbarPos("CrMin", adjRngWName, int(YCrCb_lower.val[0]));
    setTrackbarPos("CrMax", adjRngWName, int(YCrCb_upper.val[0]));
    setTrackbarPos("CbMin", adjRngWName, int(YCrCb_lower.val[1]));
    setTrackbarPos("CbMax", adjRngWName, int(YCrCb_upper.val[1]));
    setTrackbarPos("YMin", adjRngWName, int(YCrCb_lower.val[2]));
    setTrackbarPos("YMax", adjRngWName, int(YCrCb_upper.val[2]));
}

void PaperKeyboard::adjustKeyboard(Mat &img) {
    if (keys.size() == keysVec.size())
        return;

    namedWindow(adjKbWName);

    auto mouseCallback = [](int event, int x, int y, int, void *data) {
        vector<Point> &tmpPoints = *(static_cast<vector<Point> *>(data));
        switch (event) {
            case CV_EVENT_LBUTTONDOWN:
                tmpPoints.push_back(Point(x, y));
                break;
        };
    };
    setMouseCallback(adjKbWName, mouseCallback, &tmpPoints);
    if (tmpPoints.size() == 4) {
        addKey(tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3],
               keysVec[keys.size()]);
        tmpPoints.clear();
    }

    putText(img, keys.size() != keysVec.size() ? keysVec[keys.size()] : "end", Point(10, 100),
            FONT_HERSHEY_DUPLEX, 1, Scalar(0, 143, 143), 2);

    for (const Point &p : tmpPoints) {
        circle(img, p, 3, Scalar(0, 0, 0));
    }
    imshow(adjKbWName, img);

    if (keys.size() == keysVec.size())
        destroyWindow(adjKbWName);
}

void PaperKeyboard::addKey(Point x1, Point x2, Point y1, Point y2, string text) {
    keys.push_back(PKBKey(x1, x2, y1, y2, text));
}

void PaperKeyboard::addKeysByVec(Point x1, Size ksize) {
    int i = 1;
    int r = 1;
    int lX = x1.x;
    for (string &s : keysVec) {
        auto fN = s.find("\n");
        if (fN != -1) {
            r++;
            i = 1;
            lX = x1.x;
            s.replace(fN, fN + 2, "");
            continue;
        }
        int aW = s.size() > 1 ? fontSize.width * (s.size() + 1) : 0;
        Point X1(lX, x1.y + ksize.width * r);
        Point X2(X1.x + ksize.width + aW, X1.y);
        addKey(X1, X2, Point(X1.x, X1.y + ksize.height),
               Point(X2.x, X1.y + ksize.height), s);
        i++;
        lX = X2.x;
    }
}

void PaperKeyboard::deleteKeysByText(string text, int num) {
    int n = 0;
    for (int i = 0; i < keys.size(); i++) {
        if (n == num)
            break;
        if (keys[i].text == text) {
            keys.erase(keys.begin() + i);
            n++;
        }
    }
}

PKBKey PaperKeyboard::getKeyByPoint(Point p) {
    for (PKBKey k : keys) {
        if (k.x1.x < p.x && k.x2.x > p.x &&
            k.x1.y < p.y && k.y1.y > p.y)
            return k;
    }
    return PKBKey();
}

void PaperKeyboard::setOnclick(function<void(const Point &, const PKBKey &)> f) {
    onClickSet = true;
    onClick = f;
}

void PaperKeyboard::callOnclick(const Point &p, const PKBKey &k, bool runAsync) {
    if (!onClickSet)
        return;
    if (runAsync)
        async(launch::async, onClick, p, k);
    else
        onClick(p, k);
}

void PaperKeyboard::getClicks() {
    if (!hd.hands.empty()) {
        Finger &cf = hd.hands[0].farthestFinger;
        if (cf.ok) {
            int diffDist = lastDist - (int) getDist(cf.ptFar, cf.ptStart);
            if (diffDist > minDistChange && diffDist < maxDistChange) {
                PKBKey k = getKeyByPoint(Point(cf.ptStart.x + 10, cf.ptStart.y));
                if (k.x1.x != -1) {
                    if (time(nullptr) - lastClickTime >= clickDelay) {
                        callOnclick(cf.ptStart, k);
                        lastClickTime = time(nullptr);
                    }
                }
            }
        }
    }
}

void PaperKeyboard::drawKeys(Mat &img, Scalar color) {
    for (PKBKey &k : keys)
        k.draw(img, color, fontSize);
}

void PaperKeyboard::draw(Mat &img, Scalar color) {
    drawKeys(img, color);
    hd.drawHands(img, color);
}