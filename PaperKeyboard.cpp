#include "PaperKeyboard.h"

namespace PaperKeyboard {

    PrintType::PrintType(bool gaugeLine, QRCodePos qrCOdePos) : gaugeLine(gaugeLine), qrCOdePos(qrCOdePos) {}

    PrintType::PrintType(string str) {
        vector<string> splits;
        split(str, splits, ' ');
        if (splits.size() != 2)
            return;

        gaugeLine = splits[0] == "1";
        qrCOdePos = QRCodePos(safeStoi(splits[1]));
    }

    string PrintType::serialize() {
        string res;
        if (gaugeLine)
            res += "1 ";
        else
            res += "0 ";
        res += to_string(qrCOdePos);
        return res;
    }


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

    void PaperKeyboard::adjustKeyboardByQR(Mat img) {
        vector<Decoded_QRCode> decoded;
        decodeQr(move(img), decoded);
        for (const Decoded_QRCode &q : decoded) {
            deserializeFromString(q.data, q.location[2]);
        }
    }

    void PaperKeyboard::addKey(Point x1, Point x2, Point y1, Point y2, KeyType type, string text) {
        keys.emplace_back(x1, x2, y1, y2, type, text);
    }

    vector<vector<Point>> PaperKeyboard::getKeysPositions(Point x1, Size ksize) {
        vector<vector<Point>> res;
        int i = 1;
        int r = 0;
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
            Point X1(lX, x1.y + r > 0 ? ksize.height * r : 0);
            Point X2(X1.x, X1.y);
            vector<string> splits;
            if (split(s, splits, PKB_STR_TYPE_CHANGE) > 1) {
                if (splits.size() > 2)
                    X2.x += safeStoi(splits[2]);
                else
                    X2.x += ksize.width;
            } else
                X2.x += s.size() > 1 ? fontSize.width * (s.size() + 1) : 0 + ksize.width;

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
            KeyType type = BUTTON;
            vector<string> splits;
            if (split(s, splits, PKB_STR_TYPE_CHANGE) > 1) {
                if (splits[1] == "1")
                    type = SLIDEBAR;
            }
            addKey(keysPositions[i][0], keysPositions[i][1], keysPositions[i][2], keysPositions[i][3],
                   type, splits.size() > 1 ? "0" : s);
            i++;
        }
    }

    void PaperKeyboard::deleteKeysByText(string text, int num) {
        int n = 0;
        for (int i = 0; i < keys.size(); i++) {
            if (num > 0 && n == num)
                break;
            if (keys[i].getVal() == text) {
                keys.erase(keys.begin() + i);
                n++;
            }
        }
    }

    Key *PaperKeyboard::getKeyByPoint(Point p) {
        for (Key &k : keys) {
            if (k.x1.x < p.x && k.x2.x > p.x &&
                k.x1.y < p.y && k.y1.y > p.y)
                return &k;
        }
        return nullptr;
    }

    void PaperKeyboard::setOnclick(function<void(const Point &, Key &)> f) {
        onClickSet = true;
        onClick = move(f);
    }

    void PaperKeyboard::callOnclick(const Point &p, Key &k, bool runAsync) {
        if (!onClickSet)
            return;
//        if (runAsync)
//            async(launch::async, onClick, p, k);
//       else
        onClick(p, k);
    }

    void PaperKeyboard::getClicks() {
        if (hd.lastHands.empty())
            return;

        for (Hand &h : hd.hands) {
            ShortHand lastH = h.getSame(hd.lastHands);

            for (int j = 0; j < h.fingers.size(); j++) {
                Finger cf;
                if (clrf == FARTHEST)
                    cf = h.farthestFinger;
                else if (clrf == HIGHER)
                    cf = h.higherFinger;
                else if (clrf == ALL)
                    cf = h.fingers[j];

                ShortFinger lastF = cf.getSame(lastH.fingers);
                int diffDist = abs(getDist(lastF.ptFar, lastF.ptStart) - getDist(cf.ptFar, cf.ptStart));
                if (diffDist > minDistChange && diffDist < maxDistChange) {
                    Key *k = getKeyByPoint(Point(cf.ptStart.x + 10, cf.ptStart.y));
                    if (k == nullptr)
                        return;
                    if (k->x1.x != -1) {
                        if (time(nullptr) - lastClickTime >= clickDelay) {
                            callOnclick(cf.ptStart, *k);
                            lastClickTime = time(nullptr);
                        }
                    }
                }
                if (clrf != ALL) break;
            }
        }
    }

    void PaperKeyboard::drawKeys(Mat &img, Scalar color) {
        for (Key &k : keys)
            k.draw(img, color, fontSize);
    }

    void PaperKeyboard::draw(Mat &img, Scalar color) {
        drawKeys(img, color);
        hd.drawHands(img, color);
    }

    string PaperKeyboard::serialize2str() {
        string res;
        res += string(PKB_HEADER) + "\n";
        res += printType.serialize() + "\n";
        res += to_string(GaugeLineLength) + "\n";
        if (printType.gaugeLine && printType.qrCOdePos != QRCodePos::TOP_LEFT)
            res += to_string(GL_KB_INDENT) + "\n";

        for (const Key &k : keys) {
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
        printType = PrintType(splits[1]);
        int sclLineLength = safeStoi(splits[2]);
        double scale = 1;
        double angle = 1;
        int indent = 0;
        if (printType.gaugeLine && printType.qrCOdePos != QRCodePos::TOP_LEFT) {
            indent = safeStoi(splits[3]);
            keysStartLine++;
        }

        if (!scaleLine.empty() && printType.gaugeLine) {
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
            if (splits2[0] == KEY_TYPE_BUTTON)
                cordsStart++;
            vector<int> cords;
            for (int i = cordsStart; i < splits2.size(); i++) {
                cords.emplace_back(safeStoi(splits2[i]));
            }
            for (int i = 0; i < cords.size(); i += 2) {
                cords[i] /= scale;
                cords[i + 1] /= scale;
                if (!printType.gaugeLine) {
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
            string text = splits2[0] == KEY_TYPE_BUTTON ? splits2[1] : "";
            addKey(Point(cords[0], cords[1]),
                   Point(cords[2], cords[3]),
                   Point(cords[4], cords[5]),
                   Point(cords[6], cords[7]), getKeyType(splits2[0]), text);
        }
        return true;
    }

    void PaperKeyboard::prepare4Print(Mat &img) {
        resize(img, img, Size(A4_SIZE.height, A4_SIZE.width));

        Mat qr(190, 190, CV_8UC1, COLOR_WHITE);
        if (printType.qrCOdePos != NONE) {
            encodeQr(qr, serialize2str());
            if (printType.qrCOdePos == BTM_RIGHT)
                qr.copyTo(img(Rect(img.cols - qr.cols, img.rows - qr.rows, qr.cols, qr.rows)));
            else
                qr.copyTo(img(Rect(0, 0, qr.cols, qr.rows)));
        }

        Point glStPt(GL_KB_INDENT, GL_KB_INDENT);
        if (printType.gaugeLine) {
            if (printType.qrCOdePos == TOP_LEFT) {
                glStPt.x += qr.cols;
            }
            line(img, glStPt, Point(glStPt.x + GaugeLineLength, glStPt.y), COLOR_BLACK, 2);
        }

        Mat keysIm(img.rows, img.cols, CV_8UC1, COLOR_WHITE);
        if (printType.qrCOdePos != NONE)
            resize(keysIm, keysIm, Size(keysIm.cols - qr.cols, keysIm.rows - qr.rows));
        if (printType.gaugeLine && printType.qrCOdePos != TOP_LEFT)
            resize(keysIm, keysIm, Size(keysIm.cols - glStPt.x, keysIm.rows - glStPt.y - GL_KB_INDENT));
        if (!printType.gaugeLine && printType.qrCOdePos != TOP_LEFT)
            resize(keysIm, keysIm, Size(keysIm.cols - GL_KB_INDENT, keysIm.rows - GL_KB_INDENT));

        drawKeys(keysIm, COLOR_BLACK);

        if (printType.qrCOdePos == TOP_LEFT)
            keysIm.copyTo(img(Rect(qr.cols, qr.rows, keysIm.cols, keysIm.rows)));
        else if (printType.gaugeLine)
            keysIm.copyTo(img(Rect(glStPt.x, glStPt.y + GL_KB_INDENT, keysIm.cols, keysIm.rows)));
        else
            keysIm.copyTo(img(Rect(GL_KB_INDENT, GL_KB_INDENT, keysIm.cols, keysIm.rows)));

    }

    bool PaperKeyboard::save2file(string filePath) {
        ofstream file;
        file.open(filePath);
        file << serialize2str();

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
}