#include "PaperKeyboard.h"

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
        if (event == CV_EVENT_LBUTTONDOWN) {
            tmpPoints.push_back(Point(x, y));
        }
    };
    setMouseCallback(adjKbWName, mouseCallback, &tmpPoints);
    if (tmpPoints.size() == 4) {
        keysPositions.push_back(vector<Point>{tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3]});
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

vector<vector<Point>> PaperKeyboard::getKeysPositions(Point x1, Size ksize) {
    vector<vector<Point>> res;
    int i = 1;
    int r = 1;
    int lX = x1.x;
    for (string &s : keysVec) {
        auto fN = s.find('\n');
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
        res.push_back(vector<Point>{X1, X2, Point(X1.x, X1.y + ksize.height),
                                    Point(X2.x, X1.y + ksize.height)});
        i++;
        lX = X2.x;
    }
    return res;
}

void PaperKeyboard::addKeysByVec(Point x1, Size ksize) {
    int i = 0;
    if (keysPositions.empty() || keysPositions.size() < keysVec.size())
        keysPositions = getKeysPositions(x1, ksize);

    for (const string &s : keysVec) {
        addKey(keysPositions[i][0], keysPositions[i][1], keysPositions[i][2], keysPositions[i][3], s);
        i++;
    }
}

void PaperKeyboard::deleteKeysByText(string text, int num) {
    int n = 0;
    for (int i = 0; i < keys.size(); i++) {
        if (num > 0 && n == num)
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

void PaperKeyboard::adjustKeyboardByQR(Mat img) {
    vector<Decoded_QRCode> decoded;
    decodeQr(img, decoded);
    vector<string> splits; // split string into lines
    vector<string> splits2; // split lines by space
    for (const Decoded_QRCode &q : decoded) {
        split(q.data, splits, '\n');
        if (splits[0] != "%PKB%")
            break;
        for (auto sp = splits.begin() + 2; sp != splits.end(); ++sp) {
            split(*sp, splits2, ' ');
            if (splits2.size() != 9)
                break;
            addKey(Point(stoi(splits2[1]), stoi(splits2[2])),
                   Point(stoi(splits2[3]), stoi(splits2[4])),
                   Point(stoi(splits2[5]), stoi(splits2[6])),
                   Point(stoi(splits2[7]), stoi(splits2[8])), splits2[0]);

        }
        splits2.clear();
    }
}

string PaperKeyboard::serialize2str() {
    string res;
    int i = 0;

    res += "%PKB%\n";
    res += to_string(GaugeLineLength) + "\n";

    for (const vector<Point> &p : keysPositions) {
        res += keysVec[i] + " ";
        res += to_string(p[0].x) + " ";
        res += to_string(p[0].y) + " ";
        res += to_string(p[1].x) + " ";
        res += to_string(p[1].y) + " ";
        res += to_string(p[2].x) + " ";
        res += to_string(p[2].y) + " ";
        res += to_string(p[3].x) + " ";
        res += to_string(p[3].y) + "\n";
        i++;
    }

    return res;
}

void PaperKeyboard::prepare4Print(Mat &img) {
    resize(img, img, Size(A4_SIZE.height, A4_SIZE.width));
    Mat qr(150, 150, CV_8UC1, Scalar(255, 255, 255));
    encodeQr(qr, serialize2str());
    qr.copyTo(img(Rect(0, 0, qr.cols, qr.rows)));
    line(img, Point(qr.cols, 10), Point(qr.cols + GaugeLineLength, 10), Scalar(0, 0, 0), 2);
    drawKeys(img, Scalar(0, 0, 0));
    imshow("i", img);
}