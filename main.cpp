#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "PaperKeyboard.h"

#include <SerialStream.h>

#include "GUI.hpp"
#define CVUI_IMPLEMENTATION
#include "cvui.h"

using namespace std;
using namespace cv;
using namespace LibSerial;
using namespace PaperKeyboard;

SerialStream serial;

bool openSerial(string port) {
    serial.Open(move(port));
    serial.SetCharSize(SerialStreamBuf::CHAR_SIZE_8);
    serial.SetBaudRate(SerialStreamBuf::BaudRateEnum::BAUD_9600);
    serial.SetNumOfStopBits(1);
    serial.SetFlowControl(SerialStreamBuf::FLOW_CONTROL_NONE);
    return serial.good();
}

bool writeSerial(const string &str) {
    if (serial.good()) {
        usleep(5000);
    } else {
        return false;
    }
    serial << str;
    usleep(100000);
    return true;
}

void flipImg(const Mat &img, Mat &res) {
    flip(img, res, 0);
    flip(img, res, 1);
}

void onClick(const Point &p, Key &k) {
    string val = k.getVal(p);
    cout << val << endl;
    writeSerial(val);
}

bool should_adjustManually = false;
bool should_adjustScale = false;
bool should_showPrint = false;

int main(int argc, char **argv) {
    string currentDir = "/home/prostoichelovek/projects/paper_keyboard/";
    if (argc > 1)
        currentDir = argv[1];

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Unable to open video capture" << endl;
        return EXIT_FAILURE;
    }

    string portDev = "/dev/ttyUSB0";
    if (!openSerial(portDev)) {
        cerr << "Unable to open serial port " << portDev << endl;
    } else {
        cout << "Serial port " << portDev << " opened successfully" << endl;
    }

    PaperKeyboard::PaperKeyboard pk;

    /*pk.keysVec = vector<string>{
         "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
         "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
         "z", "x", "c", "v", "b", "n", "m", "\n",
         "_", "<-"};*/
    /*pk.keysVec = vector<string>{
         "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
         "й", "ц", "у", "к", "е", "н", "г", "ш", "щ", "з", "х", "ъ", "\n",
         "ф", "ы", "в", "а", "п", "р", "о", "л", "д", "ж", "э", "\n",
         "я", "ч", "с", "м", "и", "т", "ь", "б", "ю", "\n",
         "_", "<-"
    };*/
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

    cap >> pk.bg;
    flipImg(pk.bg, pk.bg);
    while (cap.isOpened()) {
        char key = -1;
        cap >> frame;
        flipImg(frame, frame);
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

        // handling keystrokes
        if (key == -1)
            key = waitKey(1);
        if (key != -1) {
            switch (key) {
                case 'q': // exit
                    cout << "exit" << endl;
                    return EXIT_SUCCESS;
                case 'b': // change bg
                    cap >> pk.bg;
                    flipImg(pk.bg, pk.bg);
                    cout << "Background changed." << endl;
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