//
// Created by prostoichelovek on 04.02.19.
//

#ifndef PAPERKEYBOARD_GUI_HPP
#define PAPERKEYBOARD_GUI_HPP

#include <vector>
#include <string>

#include <opencv2/opencv.hpp>

#include "cvui.h"
#include "EnhancedWindow.h"

#include "PaperKeyboard.h"

using namespace std;
using namespace cv;

namespace PaperKeyboard {
    namespace GUI {
        String adjKbWName = "Adjust keyboard";
        String adjScWName = "adjust scale";
        String printKbWName = "Print";
        String settingsWName = "Settings";
        int width = 200;
        Mat settingsWin(660, width * 3 + 100, CV_8UC1);
        EnhancedWindow printSettings(20, 80, 110, 150, "Settings");
        EnhancedWindow adjustKeyboardWin(20, 80, 150, 125, "Select key");
        int currentKey = 0;
        vector<Point> tmpPoints;
        vector<Point> tmpPoints2;
        vector<int> selectedKeys;

        void init() {
            namedWindow(settingsWName);
            cvui::init(settingsWName);
        }

        void displaySettingsGUI(PaperKeyboard &pk, char &key) {
            cvui::context(settingsWName);
            settingsWin = Scalar(49, 52, 49);
            cvui::beginColumn(settingsWin, 20, 20, width, -1, 6);

            cvui::text("blur kernel size");
            cvui::trackbar(width, &pk.hd.blurKsize.width, 1, 100);
            cvui::space(5);

            cvui::text("threshold sens val");
            cvui::trackbar(width, &pk.hd.thresh_sens_val, 1, 100);
            cvui::space(5);

            cvui::text("Max angle");
            cvui::trackbar(width, &pk.hd.maxAngle, 20, 180);
            cvui::space(5);

            cvui::text("Min distance change for click");
            cvui::trackbar(width, &pk.minDistChange, 0, 200);
            cvui::space(5);

            cvui::text("Max distance change for click");
            cvui::trackbar(width, &pk.maxDistChange, pk.minDistChange + 1, 350);
            cvui::space(5);

            cvui::text("Click delay");
            cvui::trackbar(width, &pk.clickDelay, 0.1f, 5.f);
            cvui::space(5);

            cvui::text("Finger");
            cvui::beginRow(settingsWin, 20, 505, width);
            if (cvui::button("Farthest"))
                pk.clrf = FARTHEST;
            if (cvui::button("Higher"))
                pk.clrf = HIGHER;
            if (cvui::button("All"))
                pk.clrf = ALL;
            cvui::endRow();
            cvui::space(3);

            cvui::checkbox("Blur", &pk.hd.shouldBlur);
            cvui::space(2);

            cvui::checkbox("Check size", &pk.hd.shouldCheckSize);
            cvui::space(2);

            cvui::checkbox("Check angles", &pk.hd.shouldCheckAngles);
            cvui::space(2);

            cvui::checkbox("Get last fingers", &pk.hd.shouldGetLast);
            cvui::space(2);
            cvui::endColumn();

            cvui::beginColumn(settingsWin, width + 50, 20, width, -1, 6);
            cvui::text("CrMin");
            cvui::trackbar(width, &pk.YCrCb_lower[0], 0., 255.);
            cvui::text("CrMax");
            cvui::trackbar(width, &pk.YCrCb_upper[0], 0., 255.);
            cvui::space(5);
            cvui::text("CbMin");
            cvui::trackbar(width, &pk.YCrCb_lower[1], 0., 255.);
            cvui::text("CbMax");
            cvui::trackbar(width, &pk.YCrCb_upper[1], 0., 255.);
            cvui::space(5);
            cvui::text("YMin");
            cvui::trackbar(width, &pk.YCrCb_lower[2], 0., 255.);
            cvui::text("YMax");
            cvui::trackbar(width, &pk.YCrCb_upper[2], 0., 255.);
            cvui::space(5);
            cvui::endColumn();

            cvui::beginColumn(settingsWin, (width + 50) * 2, 20, width, -1, 6);
            if (cvui::button("Exit (q)"))
                key = 'q';
            cvui::space(2);
            if (cvui::button("Change background (b)"))
                key = 'b';
            cvui::space(2);
            if (cvui::button("Adjust by QR (r)"))
                key = 'r';
            cvui::space(2);
            if (cvui::button("Adjust manually (m)"))
                key = 'm';
            cvui::space(2);
            if (cvui::button("Load from file (l)"))
                key = 'l';
            cvui::space(2);
            if (cvui::button("Save to file (s)"))
                key = 's';
            cvui::space(2);
            if (cvui::button("Adjust scale (c)"))
                key = 'c';
            cvui::space(2);
            if (cvui::button("Print (p)"))
                key = 'p';
            cvui::space(2);
            if (cvui::button("Flip horizontally (h)"))
                key = 'h';
            cvui::space(2);
            if (cvui::button("Flip vertically (v)"))
                key = 'v';
            cvui::endColumn();
            cvui::update();
            cv::imshow(settingsWName, settingsWin);
        }

        bool shouldShowPrintGUI = true;
        void displayPrintGUI(PaperKeyboard &pk) {
            Mat i(A4_SIZE, CV_8UC1, Scalar(255, 255, 255));
            namedWindow(printKbWName);
            pk.prepare4Print(i);
            if (shouldShowPrintGUI) {
                cvui::context(printKbWName);
                printSettings.begin(i);
                if (!printSettings.isMinimized()) {
                    cvui::checkbox("Gauge line", &pk.printType.gaugeLine);
                    cvui::space(5);
                    cvui::text("QR position");
                    cvui::space(3);
                    if (cvui::button(90, 25, "Top Left"))
                        pk.printType.qrCOdePos = TOP_LEFT;
                    cvui::space(2);
                    if (cvui::button(90, 25, "Btm Right"))
                        pk.printType.qrCOdePos = BTM_RIGHT;
                    cvui::space(2);
                    if (cvui::button(90, 25, "None"))
                        pk.printType.qrCOdePos = NONE;
                }
                printSettings.end();
                cvui::update(printKbWName);
            }
            imshow(printKbWName, i);
        }

        bool editing = false;
        bool rAngles = false;
        int keyWidth = 0;
        int keyHeight = 0;
        string keyCmd;
        void displayAdjustManuallyGUI(PaperKeyboard &pk, vector<string> &keysVec, Mat &img) {
            if (pk.keys.size() < keysVec.size())
                pk.keys.resize(keysVec.size());
            namedWindow(adjKbWName);
            cvui::context(adjKbWName);
            if (cvui::mouse(cvui::LEFT_BUTTON, cvui::CLICK)) {
                if (!cvui::mouse().inside(Rect(adjustKeyboardWin.mX, adjustKeyboardWin.mY,
                                               adjustKeyboardWin.mWidth, adjustKeyboardWin.mHeight))) {
                    tmpPoints.emplace_back(cvui::mouse().x, cvui::mouse().y);
                }
            }
            adjustKeyboardWin.begin(img);
            int inRow = 0;
            int rowLength = 3;
            int i = 0;
            int rowsNum = 1;
            for (string &s : keysVec) {
                if (inRow == 0) {
                    cvui::beginRow();
                    rowsNum++;
                }
                if (cvui::button(s)) {
                    currentKey = i;
                }
                inRow++;
                if (inRow >= rowLength || s.size() > 2) {
                    cvui::endRow();
                    inRow = 0;
                }
                i++;
            }
            if (inRow != 0)
                cvui::endRow();
            cvui::space(2);
            adjustKeyboardWin.mHeightNotMinimized = 125 + (25 * rowsNum);
            string crnKeyS = keysVec[currentKey];
            vector<string> splits;
            if (split(keysVec[currentKey], splits, PKB_STR_TYPE_CHANGE) > 1) {
                if (splits[1] == "1")
                    crnKeyS = "slidebar";
            }
            cvui::text("Current: " + crnKeyS);
            cvui::space(6);
            string keyText;
            int keyType = 0;
            cvui::checkbox("Right angles", &rAngles);
            cvui::space(2);
            if (cvui::button("Add")) {
                cout << "Text: ";
                std::getline(std::cin >> std::ws, keyText);
                cout << "Type(0,1): ";
                cin >> keyType;
                cout << "OnCLick cmd(or -): ";
                std::getline(std::cin >> std::ws, keyCmd);
                if (keyCmd == "-")
                    keyCmd.clear();
                if (rAngles) {
                    cout << "Width: ";
                    cin >> keyWidth;
                    cout << "Height: ";
                    cin >> keyHeight;
                }
                if (keyType == SLIDEBAR)
                    keyText = PKB_STR_TYPE_CHANGE + to_string(keyType) + PKB_STR_TYPE_CHANGE + keyText;
                keysVec.emplace_back(keyText);
                currentKey = keysVec.size() - 1;
                tmpPoints.clear();
                editing = true;
                if (rAngles)
                    cout << "Select first point of the key" << endl;
                else
                    cout << "Select key points" << endl;
            }
            if (cvui::button("Edit")) {
                tmpPoints.clear();
                if (rAngles) {
                    cout << "Width: ";
                    cin >> keyWidth;
                    cout << "Height: ";
                    cin >> keyHeight;
                    cout << "OnCLick cmd(or -): ";
                    std::getline(std::cin >> std::ws, keyCmd);
                    if (keyCmd == "-")
                        keyCmd.clear();
                    editing = true;
                    cout << "Select first point of the key" << endl;
                }
            }
            adjustKeyboardWin.end();
            cvui::update(adjKbWName);
            if (tmpPoints.size() == 4
                || (tmpPoints.size() == 1 && rAngles && editing)) {
                if (rAngles && editing) {
                    Point fP = tmpPoints[0];
                    tmpPoints.emplace_back(fP.x + keyWidth, fP.y);
                    tmpPoints.emplace_back(fP.x, fP.y + keyHeight);
                    tmpPoints.emplace_back(fP.x + keyWidth, fP.y + keyHeight);
                }
                pk.keysPositions.emplace_back(vector<Point>{tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3]});
                KeyType type = BUTTON;
                vector<string> splits;
                if (split(keysVec[currentKey], splits, PKB_STR_TYPE_CHANGE) > 1) {
                    if (splits[1] == "1")
                        type = SLIDEBAR;
                }
                pk.keys[currentKey] = Key(tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3],
                                          type, type == SLIDEBAR ? "0" : keysVec[currentKey]);
                pk.keys[pk.keys.size() - 1].onClickCmd = keyCmd;
                if (find(selectedKeys.begin(), selectedKeys.end(), currentKey) == selectedKeys.end()) {
                    selectedKeys.emplace_back(currentKey);
                }
                tmpPoints.clear();
                if (rAngles && editing) {
                    keyWidth = 0;
                    keyHeight = 0;
                    rAngles = false;
                    editing = true;
                }
            }
            if (find(selectedKeys.begin(), selectedKeys.end(), currentKey) != selectedKeys.end()) {
                pk.keys[currentKey].draw(img, Scalar(0, 255, 255));
            }
            for (const Point &p : tmpPoints) {
                circle(img, p, 3, Scalar(0, 255, 255), LineTypes::FILLED);
            }
            imshow(adjKbWName, img);
        }

        bool displayAdjustScaleGUI(PaperKeyboard &pk, Mat &img) {
            namedWindow(adjScWName);
            auto mouseCallback = [](int event, int x, int y, int, void *data) {
                vector<Point> &tmpPoints2 = *(static_cast<vector<Point> *>(data));
                if (event == MouseEventTypes::EVENT_LBUTTONDOWN) {
                    tmpPoints2.emplace_back(x, y);
                }
            };
            setMouseCallback(adjScWName, mouseCallback, &tmpPoints2);

            for (const Point &p : tmpPoints2) {
                circle(img, p, 3, Scalar(255, 0, 255), LineTypes::FILLED);
            }
            imshow(adjScWName, img);

            if (tmpPoints2.size() == 2) {
                pk.scaleLine.swap(tmpPoints2);
                destroyWindow(adjScWName);
                tmpPoints2.clear();
                return true;
            }
            return false;
        }
    }
}

#endif //PAPERKEYBOARD_GUI_HPP
