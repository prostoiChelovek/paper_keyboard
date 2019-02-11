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
        bgs = createBackgroundSubtractorMOG2();
        bgs->setShadowValue(0);
        bgs->setShadowThreshold(0.01);
    }

    Mat PaperKeyboard::detectHands(Mat img) {
        Mat imgYCrCb, mask;
        deleteBg(img, img, bgs, bgs_learn, 127);

        cvtColor(img, imgYCrCb, COLOR_BGR2YCrCb);
        mask = hd.detectHands_range(imgYCrCb, YCrCb_lower, YCrCb_upper);

        hd.getFingers();
        hd.getCenters();
        hd.getHigherFingers();
        hd.getFarthestFingers();

        return mask;
    }

    void PaperKeyboard::updateBG() {
        if (bgs_learnNFrames > 0)
            bgs_learnNFrames--;
        else if (bgs_learn) {
            bgs_learn = false;
            cout << "Background learned" << endl;
        }
    }

    void PaperKeyboard::adjustKeyboardByQR(Mat img) {
        vector<Decoded_QRCode> decoded;
        decodeQr(move(img), decoded);
        for (const Decoded_QRCode &q : decoded) {
            praseString(q.data, q.location[2]);
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

    string PaperKeyboard::serializeKeys2str() {
        string res;

        Point startPtX, startPtY;
        Point endPtX, endPtY;
        vector<vector<Point>> lines;
        for (int i = 0; i < keys.size(); i++) {
            Key &k = keys[i];
            startPtX = k.x1;
            startPtY = k.y1;
            for (int j = i + 1; j < keys.size(); j++) {
                Key &k2 = keys[j];
                if (k2.x1.y != startPtX.y || k2.y1.y != startPtY.y) {
                    endPtX = keys[j - 1].x2;
                    endPtY = keys[j - 1].y2;
                    i = j - 1;
                    break;
                }
            }
            if (i == keys.size() - 1) {
                endPtX = k.x2;
                endPtY = k.y2;
            }
            lines.emplace_back(vector<Point>{startPtX, startPtY, endPtX, endPtY});
        }

        int currentLine = 0;
        for (Key &k : keys) {
            res += k.type == BUTTON ? KEY_TYPE_BUTTON : KEY_TYPE_SLIDEBAR;
            res += " ";
            if (k.type != SLIDEBAR) res += k.getVal() + " "; else res += "0 ";
            if (k.x1 == lines[currentLine][0] && k.y1 == lines[currentLine][1]) {
                res += to_string(k.x1.x) + " ";
                res += to_string(k.x1.y) + " ";
                res += to_string(k.y1.x) + " ";
                res += to_string(k.y1.y);
                if (k.x2 == lines[currentLine][2] && k.y2 == lines[currentLine][3])
                    res += " ";
            }
            if (k.x2 == lines[currentLine][2] && k.y2 == lines[currentLine][3]) {
                res += to_string(k.x2.x) + " ";
                res += to_string(k.x2.y) + " ";
                res += to_string(k.y2.x) + " ";
                res += to_string(k.y2.y);
                currentLine++;
            }
            if (currentLine == lines.size())
                break;
            res += DATA_SEPARATOR;
        }
        return res;
    }

    string PaperKeyboard::serialize2str() {
        string res;
        res += string(PKB_HEADER) + DATA_SEPARATOR;
        res += printType.serialize() + DATA_SEPARATOR;
        res += to_string(GaugeLineLength) + DATA_SEPARATOR;
        if (printType.gaugeLine && printType.qrCOdePos != TOP_LEFT)
            res += to_string(GL_KB_INDENT) + DATA_SEPARATOR;
        res += serializeKeys2str();
        return res;
    }

    bool PaperKeyboard::praseString(string str, Point startPoint) {
        vector<string> splits;
        split(str, splits, DATA_SEPARATOR);
        if (splits.size() < 3) return false;
        if (splits[0] != PKB_HEADER)
            return false;
        int keysStartLine = 3;
        printType = PrintType(splits[1]);
        int sclLineLength = safeStoi(splits[2]);
        double scale = 1;
        double angle = 1;
        int indent = 0;
        if (printType.gaugeLine && printType.qrCOdePos != TOP_LEFT) {
            indent = safeStoi(splits[3]);
            keysStartLine++;
        }

        if (!scaleLine.empty() && printType.gaugeLine) {
            scale = sclLineLength / getDist(scaleLine[0], scaleLine[1]);
            angle = getAngle(scaleLine[1], scaleLine[0]);
            cout << scale << " " << angle << endl;
        }
        Point startPtX(-1, -1), startPtY;
        Point endPtX, endPtY;
        vector<vector<Point>> lines;
        vector<Size> keySizes;
        int n = 1;
        for (auto sp = splits.begin() + keysStartLine; sp != splits.end(); ++sp) {
            vector<string> splits2;
            split(*sp, splits2, ' ');
            if (splits2.size() < 2)
                break;
            if (startPtX.x != -1)
                n++;
            vector<int> cords1;
            if (splits2.size() == 6 || splits2.size() == 10) {
                Point &stPt = startPoint;
                if (printType.gaugeLine) {
                    if (scaleLine.empty()) {
                        cerr << "Please pick out gauge line!" << endl;
                        return false;
                    }
                    stPt = scaleLine[0];
                }
                for (int i = 2; i < splits2.size(); i += 2) {
                    cords1.emplace_back(safeStoi(splits2[i]) / scale + stPt.x);
                    cords1.emplace_back(safeStoi(splits2[i + 1]) / scale + stPt.y + indent);
                }
            }
            if (splits2.size() == 6) {
                if (startPtX.x == -1) {
                    startPtX.x = cords1[0];
                    startPtX.y = cords1[1];
                    startPtY.x = cords1[2];
                    startPtY.y = cords1[3];
                } else {
                    endPtX.x = cords1[0];
                    endPtX.y = cords1[1];
                    endPtY.x = cords1[2];
                    endPtY.y = cords1[3];
                    lines.emplace_back(vector<Point>{startPtX, startPtY, endPtX, endPtY});
                    keySizes.emplace_back(abs(endPtX.x - startPtX.x) / n,
                                          abs(startPtY.y - startPtX.y));
                    n = 1;
                    startPtX.x = -1;
                }
            } else if (splits2.size() == 10) {
                lines.emplace_back(vector<Point>{
                        Point(cords1[0], cords1[1]),
                        Point(cords1[2], cords1[3]),
                        Point(cords1[4], cords1[5]),
                        Point(cords1[6], cords1[7]),
                });
                keySizes.emplace_back(abs(cords1[4] - cords1[0]),
                                      abs(cords1[3] - cords1[1]));
                n = 1;
            }
        }
        if (lines.empty())
            return false;

        int lineNum = 0;
        int lastX = lines[lineNum][0].x;
        bool started = false;
        n = 0;
        for (auto sp = splits.begin() + keysStartLine; sp != splits.end(); ++sp) {
            vector<string> splits2;
            split(*sp, splits2, ' ');
            if (splits2.size() < 2)
                break;
            if (splits2[0] == CMD_START_SYM && n != 0) {
                string cmd;
                for (int i = 1; i < splits2.size(); i++) {
                    cmd += splits2[i] + " ";
                }
                keys[n - 1].onClickCmd = cmd;
                continue;
            }
            if (splits2.size() == 6)
                started = !started;
            vector<Point> cords = {
                    Point(lastX, lines[lineNum][0].y),
                    Point(lastX + keySizes[lineNum].width, lines[lineNum][0].y),
                    Point(lastX, lines[lineNum][0].y + keySizes[lineNum].height),
                    Point(lastX + keySizes[lineNum].width, lines[lineNum][0].y + keySizes[lineNum].height),
            };
            string text = splits2[0] == KEY_TYPE_BUTTON ? splits2[1] : "0";
            addKey(cords[0], cords[1], cords[2], cords[3], getKeyType(splits2[0]), text);
            n++;
            lastX = cords[1].x;
            if ((splits2.size() == 6 && !started)
                || lastX == lines[lineNum][3].x) {
                lineNum++;
                if (lineNum >= lines.size())
                    break;
                lastX = lines[lineNum][0].x;
            }
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
        return praseString(data, startPoint);
    }
}