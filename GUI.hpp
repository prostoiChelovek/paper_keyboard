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
        Mat settingsWin(600, width * 3 + 100, CV_8UC1);
        EnhancedWindow printSettings(20, 80, 110, 150, "Settings");
        EnhancedWindow adjustKeyboardWin(20, 80, 150, 200, "Select key");
        int currentKey = 0;
        vector<Point> tmpPoints;
        vector<Point> tmpPoints2;
        vector<int> selectedKeys;

        void init() {
            namedWindow(settingsWName, CV_WINDOW_AUTOSIZE);
            cvui::init(settingsWName);
        }

        void displaySettingsGUI(PaperKeyboard &pk, char &key) {
            cvui::context(settingsWName);
            settingsWin = Scalar(49, 52, 49);
            cvui::beginColumn(settingsWin, 20, 20, width, -1, 6);

            cvui::checkbox("Blur", &pk.hd.shouldBlur);
            cvui::text("blur kernel size");
            cvui::trackbar(width, &pk.hd.blurKsize.width, 1, 100);
            cvui::space(5);

            cvui::text("threshold sens val");
            cvui::trackbar(width, &pk.hd.thresh_sens_val, 1, 100);
            cvui::space(5);

            cvui::text("Min distance change for click");
            cvui::trackbar(width, &pk.minDistChange, 0, 100);
            cvui::space(5);

            cvui::text("Max distance change for click");
            cvui::trackbar(width, &pk.maxDistChange, pk.minDistChange + 1, 200);
            cvui::space(5);

            cvui::text("Click delay");
            cvui::trackbar(width, &pk.clickDelay, 0.1f, 5.f);
            cvui::space(5);

            cvui::text("Finger");
            cvui::beginRow(settingsWin, 20, 450, width);
            if (cvui::button("Farthest"))
                pk.clrf = FARTHEST;
            if (cvui::button("Higher"))
                pk.clrf = HIGHER;
            if (cvui::button("All"))
                pk.clrf = ALL;
            cvui::endRow();
            cvui::space(5);

            cvui::checkbox("Check size", &pk.hd.shouldCheckSize);
            cvui::space(5);

            cvui::checkbox("Check angles", &pk.hd.shouldCheckAngles);
            cvui::space(5);
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
            if (cvui::button("Exit"))
                key = 'q';
            cvui::space(2);
            if (cvui::button("Change background"))
                key = 'b';
            cvui::space(2);
            if (cvui::button("Adjust by QR"))
                key = 'r';
            cvui::space(2);
            if (cvui::button("Adjust manually"))
                key = 'm';
            cvui::space(2);
            if (cvui::button("Load from file"))
                key = 'l';
            cvui::space(2);
            if (cvui::button("Save to file"))
                key = 's';
            cvui::space(2);
            if (cvui::button("Adjust scale"))
                key = 'c';
            cvui::space(2);
            if (cvui::button("Print"))
                key = 'p';
            cvui::endColumn();
            cvui::update();
            cv::imshow(settingsWName, settingsWin);
        }

        void displayPrintGUI(PaperKeyboard &pk) {
            Mat i(A4_SIZE, CV_8UC1, Scalar(255, 255, 255));
            namedWindow(printKbWName);
            pk.prepare4Print(i);
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
            imshow(printKbWName, i);
        }

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
            for (string &s : keysVec) {
                if (inRow == 0)
                    cvui::beginRow();
                if (cvui::button(s)) {
                    currentKey = i;
                }
                inRow++;
                if (inRow >= rowLength) {
                    cvui::endRow();
                    inRow = 0;
                }
                i++;
            }
            if (inRow != 0)
                cvui::endRow();
            cvui::space();
            string crnKeyS = keysVec[currentKey];
            vector<string> splits;
            if (split(keysVec[currentKey], splits, PKB_STR_TYPE_CHANGE) > 1) {
                if (splits[1] == "1")
                    crnKeyS = "slidebar";
            }
            cvui::text("Current: " + crnKeyS);
            adjustKeyboardWin.end();
            cvui::update(adjKbWName);
            if (tmpPoints.size() == 4) {
                pk.keysPositions.emplace_back(vector<Point>{tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3]});
                KeyType type = BUTTON;
                vector<string> splits;
                if (split(keysVec[currentKey], splits, PKB_STR_TYPE_CHANGE) > 1) {
                    if (splits[1] == "1")
                        type = SLIDEBAR;
                }
                pk.keys[currentKey] = Key(tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3],
                                          type, type == SLIDEBAR ? "0" : keysVec[currentKey]);
                if (find(selectedKeys.begin(), selectedKeys.end(), currentKey) == selectedKeys.end()) {
                    selectedKeys.emplace_back(currentKey);
                }
                tmpPoints.clear();
            }
            if (find(selectedKeys.begin(), selectedKeys.end(), currentKey) != selectedKeys.end()) {
                pk.keys[currentKey].draw(img, Scalar(0, 255, 255));
            }
            for (const Point &p : tmpPoints) {
                circle(img, p, 3, Scalar(0, 255, 255), CV_FILLED);
            }
            imshow(adjKbWName, img);
        }

        bool displayAdjustScaleGUI(PaperKeyboard &pk, Mat &img) {
            namedWindow(adjScWName);
            auto mouseCallback = [](int event, int x, int y, int, void *data) {
                vector<Point> &tmpPoints2 = *(static_cast<vector<Point> *>(data));
                if (event == CV_EVENT_LBUTTONDOWN) {
                    tmpPoints2.emplace_back(x, y);
                }
            };
            setMouseCallback(adjScWName, mouseCallback, &tmpPoints2);

            for (const Point &p : tmpPoints2) {
                circle(img, p, 3, Scalar(255, 0, 255), CV_FILLED);
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
