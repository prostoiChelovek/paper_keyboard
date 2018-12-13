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

void PaperKeyboard::setLast() {
    lastFingers.clear();
    for (Hand &h : hd.hands) {
        vector<ShortFinger> fingers;
        Finger f;
        if (clrf == FARTHEST)
            f = h.farthestFinger;
        else if (clrf == HIGHER)
            f = h.higherFinger;
        if (clrf == ALL) {
            for (const Finger &fng : h.fingers) {
                fingers.emplace_back(ShortFinger{fng.ptStart, fng.ptEnd, fng.ptFar});
            }
        } else
            fingers.emplace_back(ShortFinger{f.ptStart, f.ptEnd, f.ptFar});
        lastFingers.emplace_back(fingers);
    }
}

void PaperKeyboard::adjustColorRanges() {
    namedWindow(adjRngWName);
    auto onTrackbarActivity = [](int val, void *data) {
        double &vNum = *(static_cast<double *>(data));
        vNum = double(val);
    };
    createTrackbar("CrMin", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_lower.val[0]);
    createTrackbar("CrMax", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_upper.val[0]);
    createTrackbar("CbMin", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_lower.val[1]);
    createTrackbar("CbMax", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_upper.val[1]);
    createTrackbar("YMin", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_lower.val[2]);
    createTrackbar("YMax", adjRngWName, nullptr, 255, onTrackbarActivity, &YCrCb_upper.val[2]);
    setTrackbarPos("CrMin", adjRngWName, int(YCrCb_lower.val[0]));
    setTrackbarPos("CrMax", adjRngWName, int(YCrCb_upper.val[0]));
    setTrackbarPos("CbMin", adjRngWName, int(YCrCb_lower.val[1]));
    setTrackbarPos("CbMax", adjRngWName, int(YCrCb_upper.val[1]));
    setTrackbarPos("YMin", adjRngWName, int(YCrCb_lower.val[2]));
    setTrackbarPos("YMax", adjRngWName, int(YCrCb_upper.val[2]));
}

void PaperKeyboard::adjustKeyboardManually(Mat &img) {
    if (keys.size() == keysVec.size())
        return;

    namedWindow(adjKbWName);

    auto mouseCallback = [](int event, int x, int y, int, void *data) {
        vector<Point> &tmpPoints = *(static_cast<vector<Point> *>(data));
        if (event == CV_EVENT_LBUTTONDOWN) {
            tmpPoints.emplace_back(x, y);
        }
    };
    setMouseCallback(adjKbWName, mouseCallback, &tmpPoints);
    if (tmpPoints.size() == 4) {
        keysPositions.emplace_back(vector<Point>{tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3]});
        addKey(tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3],
               keysVec[keys.size()]);
        tmpPoints.clear();
    }

    putText(img, keys.size() != keysVec.size() ? keysVec[keys.size()] : "end", Point(10, 100),
            FONT_HERSHEY_DUPLEX, 1, Scalar(0, 143, 143), 2);

    for (const Point &p : tmpPoints) {
        circle(img, p, 3, Scalar(0, 255, 0));
    }
    imshow(adjKbWName, img);

    if (keys.size() == keysVec.size()) {
        tmpPoints.clear();
        destroyWindow(adjKbWName);
    }
}

void PaperKeyboard::adjustKeyboardByQR(Mat img) {
    vector<Decoded_QRCode> decoded;
    decodeQr(move(img), decoded);
    for (const Decoded_QRCode &q : decoded) {
        deserializeFromString(q.data, q.location[2]);
    }
}

void PaperKeyboard::adjustScale(Mat &img) {
    if (!scaleLine.empty())
        return;
    namedWindow(adjScWName);
    auto mouseCallback = [](int event, int x, int y, int, void *data) {
        vector<Point> &tmpPoints = *(static_cast<vector<Point> *>(data));
        if (event == CV_EVENT_LBUTTONDOWN) {
            tmpPoints.emplace_back(x, y);
        }
    };
    setMouseCallback(adjScWName, mouseCallback, &tmpPoints);

    for (const Point &p : tmpPoints) {
        circle(img, p, 3, Scalar(255, 0, 0));
    }
    imshow(adjScWName, img);

    if (tmpPoints.size() == 2) {
        scaleLine.swap(tmpPoints);
        destroyWindow(adjScWName);
        tmpPoints.clear();
    }
}


void PaperKeyboard::addKey(Point x1, Point x2, Point y1, Point y2, string text) {
    keys.emplace_back(x1, x2, y1, y2, text);
}

void PaperKeyboard::addKey(Point x1, Point x2, Point y1, Point y2, pkb_key_type type, string text) {
    keys.emplace_back(x1, x2, y1, y2, type, text);
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
        Point X1(lX, x1.y + r != 1 ? ksize.height * r : 0);
        Point X2(X1.x + ksize.width + aW, X1.y);
        res.emplace_back(vector<Point>{X1, X2, Point(X1.x, X1.y + ksize.height),
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
        if (s.empty())
            continue;
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

void PaperKeyboard::setOnclick(function<void(const Point &, PKBKey &)> f) {
    onClickSet = true;
    onClick = move(f);
}

void PaperKeyboard::callOnclick(const Point &p, PKBKey &k, bool runAsync) {
    if (!onClickSet)
        return;
//    if (runAsync)
//        async(launch::async, onClick, p, k);
//    else
    onClick(p, k);
}

void PaperKeyboard::getClicks() {
    if (lastFingers.empty())
        return;

    for (int i = 0; i < hd.hands.size(); i++) {
        for (int j = 0; j < hd.hands[i].fingers.size(); j++) {
            Finger cf;
            if (clrf == FARTHEST)
                cf = hd.hands[i].farthestFinger;
            else if (clrf == HIGHER)
                cf = hd.hands[i].higherFinger;
            else if (clrf == ALL)
                cf = hd.hands[i].fingers[j];

            if (cf.ok) {
                ShortFinger last = getSame(lastFingers[i], cf);
                int diffDist = getDist(last.ptFar, last.ptStart) - getDist(cf.ptFar, cf.ptStart);
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
            if (clrf != ALL) break;
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

string PaperKeyboard::serialize2str(int printType) {
    string res;
    res += string(PKB_HEADER) + "\n";
    res += to_string(printType) + "\n";
    res += to_string(GaugeLineLength) + "\n";
    if (printType == PKB_PRINT_TYPE_4 || printType == PKB_PRINT_TYPE_2)
        res += to_string(GL_KB_INDENT) + "\n";

    for (const PKBKey &k : keys) {
        res += k.serealize2str() + "\n";
    }

    return res;
}

bool PaperKeyboard::deserializeFromString(string str, Point startPoint) {
    vector<string> splits;
    split(str, splits, '\n');
    if (splits.size() < 3) return false;
    if (splits[0] != PKB_HEADER)
        return false;
    int keysStartLine = 3;
    int mode = safeStoi(splits[1]);
    int sclLineLength = safeStoi(splits[2]);
    double scale = 1;
    double angle = 1;
    int indent = 0;
    if (mode == PKB_PRINT_TYPE_4 || mode == PKB_PRINT_TYPE_2) {
        indent = safeStoi(splits[3]);
        keysStartLine++;
    }

    if (!scaleLine.empty() && mode != PKB_PRINT_TYPE_3) {
        scale = sclLineLength / getDist(scaleLine[0], scaleLine[1]);
        angle = getAngle(scaleLine[1], scaleLine[0]);
        cout << scale << " " << angle << endl;
    }
    for (auto sp = splits.begin() + keysStartLine; sp != splits.end(); ++sp) {
        vector<string> splits2;
        split(*sp, splits2, ' ');
        if (splits2.size() < 9)
            break;
        int cordsStart = 1;
        if (splits2[0] == PKBK_TYPE_BUTTON)
            cordsStart++;
        vector<int> cords;
        for (int i = cordsStart; i < splits2.size(); i++) {
            cords.emplace_back(safeStoi(splits2[i]));
        }
        for (int i = 0; i < cords.size(); i += 2) {
            cords[i] /= scale;
            cords[i + 1] /= scale;
            if (mode != PKB_PRINT_TYPE_4 && mode != PKB_PRINT_TYPE_2) {
                cords[i] += startPoint.x;
                cords[i + 1] += startPoint.y;
            } else {
                if (scaleLine.empty()) {
                    cerr << "Please pick out gauge line!" << endl;
                    return false;
                }
                cords[i] += scaleLine[0].x;
                cords[i + 1] += scaleLine[1].y + indent;
            }
        }
        string text = splits2[0] == PKBK_TYPE_BUTTON ? splits2[1] : "";
        addKey(Point(cords[0], cords[1]),
               Point(cords[2], cords[3]),
               Point(cords[4], cords[5]),
               Point(cords[6], cords[7]), getPKBKType(splits2[0]), text);
    }
    return true;
}

void PaperKeyboard::prepare4Print(Mat &img, int printType) {
    if (printType > PKB_PRINT_TYPES_NUM) printType = PKB_PRINT_TYPES_NUM;

    resize(img, img, Size(A4_SIZE.height, A4_SIZE.width));
    Mat qr(190, 190, CV_8UC1, COLOR_WHITE);

    if (printType != PKB_PRINT_TYPE_1 && printType != PKB_PRINT_TYPE_2) {
        encodeQr(qr, serialize2str(printType));
        if (printType == PKB_PRINT_TYPE_4)
            qr.copyTo(img(Rect(img.cols - qr.cols, img.rows - qr.rows, qr.cols, qr.rows)));
        else
            qr.copyTo(img(Rect(0, 0, qr.cols, qr.rows)));
    }
    if (printType != PKB_PRINT_TYPE_1 && printType != PKB_PRINT_TYPE_3) {
        line(img, Point(10, 10), Point(10 + GaugeLineLength, 10), COLOR_BLACK, 2);
    }
    Mat keysIm(img.rows, img.cols, CV_8UC1, COLOR_WHITE);
    if (printType == PKB_PRINT_TYPE_3)
        resize(keysIm, keysIm, Size(keysIm.cols - qr.cols, keysIm.rows - qr.rows));
    else if (printType == PKB_PRINT_TYPE_2)
        resize(keysIm, keysIm, Size(keysIm.cols - 10, keysIm.rows - 10 - GL_KB_INDENT));
    else if (printType == PKB_PRINT_TYPE_4)
        resize(keysIm, keysIm, Size(keysIm.cols - qr.cols, keysIm.rows - qr.rows - 10 - GL_KB_INDENT));

    drawKeys(keysIm, COLOR_BLACK);
    if (printType == PKB_PRINT_TYPE_3)
        keysIm.copyTo(img(Rect(qr.cols, qr.rows, keysIm.cols, keysIm.rows)));
    else if (printType == PKB_PRINT_TYPE_4 || printType == PKB_PRINT_TYPE_2)
        keysIm.copyTo(img(Rect(10, 10 + GL_KB_INDENT, keysIm.cols, keysIm.rows)));
    else
        keysIm.copyTo(img(Rect(0, 0, keysIm.cols, keysIm.rows)));
}

bool PaperKeyboard::save2file(string filePath, int printType) {
    if (printType > PKB_PRINT_TYPES_NUM) printType = PKB_PRINT_TYPES_NUM;

    ofstream file;
    file.open(filePath);
    file << serialize2str(printType);

    if (file.bad()) {
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool PaperKeyboard::loadFromFile(string filePath, Point startPoint) {
    ifstream file;
    file.open(filePath);
    if (file.bad()) {
        file.close();
        return false;
    }
    string data((istreambuf_iterator<char>(file)),
                (istreambuf_iterator<char>()));
    file.close();
    return deserializeFromString(data, startPoint);
}

void PaperKeyboard::clearTmpPoints() {
    tmpPoints.clear();
}
