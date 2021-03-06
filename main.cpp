#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "PaperKeyboard.h"

#include "GUI.hpp"
#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include "SystemInteraction.hpp"

using namespace std;
using namespace cv;
using namespace PaperKeyboard;

bool should_adjustManually = false;
bool should_adjustScale = false;
bool should_showPrint = false;
bool should_flipHor = true;
bool should_flipVert = true;

void onClick(const Point &p, Key &k) {
    string val = k.getVal(p);
    cout << val << endl;
    SysInter::sendString(val);
    if (!k.onClickCmd.empty()) {
        system((k.onClickCmd + " &").c_str());
        cout << "$ " << k.onClickCmd << endl;
    }
}


void flipImg(Mat &img) {
    if (should_flipHor)
        flip(img, img, 0);
    if (should_flipVert)
        flip(img, img, 1);
}

int main(int argc, char **argv) {
    string currentDir = "/home/prostoichelovek/projects/paper_keyboard/";
    if (argc > 1)
        currentDir = argv[1];

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Unable to open video capture" << endl;
        return EXIT_FAILURE;
    }

    SysInter::init();

    PaperKeyboard::PaperKeyboard pk;


    vector<string> keysVec = vector<string>{
            "a", "b", "c", "d", "e", "f", "R", "G", "B", "\n", "\n",
            "%1%100"};
    pk.keysVec = keysVec;
    // remove \n
    for (int i = 0; i < keysVec.size(); i++) {
        auto si = keysVec[i].find('\n');
        if (si != -1)
            keysVec[i].replace(si, si + 2, "");
        if (keysVec[i].empty()) {
            keysVec.erase(keysVec.begin() + i);
            i--;
        }
    }

    pk.setOnclick(onClick);
    pk.addKeysByVec(Point(0, 0), Size(45, 45));

    GUI::init();

    Mat frame, img, mask, img2;

    while (cap.isOpened()) {
        char key = -1;
        cap >> frame;
        flipImg(frame);
        frame.copyTo(img);
        frame.copyTo(img2);

        mask = pk.detectHands(img);
        pk.getClicks();

        pk.draw(img);

        GUI::displaySettingsGUI(pk, key);

        imshow("img", img);
        imshow("mask", mask);

        if (should_adjustManually)
            GUI::displayAdjustManuallyGUI(pk, keysVec, img2);
        if (should_adjustScale) {
            if (GUI::displayAdjustScaleGUI(pk, img2)) {
                should_adjustScale = false;
                destroyWindow(GUI::adjScWName);
            }
        }
        if (should_showPrint)
            GUI::displayPrintGUI(pk);

        pk.hd.updateLast();

        pk.updateBG();

        // handling keystrokes
        if (key == -1)
            key = waitKey(1);
        if (key != -1) {
            switch (key) {
                case 'q': // exit
                    cout << "exit" << endl;
                    return EXIT_SUCCESS;
                case 'b': // change bg
                    pk.bgs_learnNFrames = 100;
                    pk.bgs_learn = true;
                    cout << "Starting learning background." << endl;
                    break;
                case 'r': // adjust keyboard by QR
                    pk.keys.clear();
                    pk.adjustKeyboardByQR(img2);
                    if (!pk.keys.empty())
                        cout << "Keyboard adjusted" << endl;
                    else
                        cerr << "Keyboard isn`t adjusted" << endl;
                    break;
                case 'm': // adjust keyboard manually
                    should_adjustManually = !should_adjustManually;
                    if (!should_adjustManually) {
                        destroyWindow(GUI::adjKbWName);
                        GUI::tmpPoints.clear();
                    } else
                        cvui::watch(GUI::adjKbWName);
                    break;
                case 'l': { // load from file
                    pk.keys.clear();
                    string filename;
                    cout << "Filename: " << flush;
                    cin >> filename;
                    if (pk.loadFromFile(currentDir + filename) && !pk.keys.empty())
                        cout << "Keyboard configuration loaded from file " << currentDir + filename << endl;
                    else
                        cerr << "Keyboard configuration isn`t loaded from file" << endl;
                }
                    break;
                case 's': { // save
                    string filename;
                    cout << "Filename: " << flush;
                    cin >> filename;
                    if (pk.save2file(currentDir + filename))
                        cout << "Keyboard configuration saved to file " << currentDir + filename << endl;
                    else
                        cerr << "Can`t save keyboard configuration to file" << endl;
                }
                    break;
                case 'c': // adjust scale
                    should_adjustScale = !should_adjustScale;
                    if (!should_adjustScale)
                        destroyWindow(GUI::adjScWName);
                    break;
                case 'p': // print
                    should_showPrint = !should_showPrint;
                    if (!should_showPrint)
                        destroyWindow(GUI::printKbWName);
                    else
                        cvui::watch(GUI::printKbWName);
                    break;
                case 'h':
                    should_flipHor = !should_flipHor;
                    break;
                case 'v':
                    should_flipVert = !should_flipVert;
                    break;
                case 'o':
                    GUI::shouldShowPrintGUI = !GUI::shouldShowPrintGUI;
                    break;
                default:
                    cout << "Key pressed: " << key << endl;
                    break;
            };
        }

    }
    cap.release();
    destroyAllWindows();
    return EXIT_SUCCESS;
}