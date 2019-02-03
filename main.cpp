#include "string"
#include "vector"

#include <opencv2/opencv.hpp>

#include "PaperKeyboard.h"

#include <SerialStream.h>

#define CVUI_IMPLEMENTATION

#include "cvui.h"

#define WINDOW_NAME "Settings"

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

int main(int argc, char **argv) {
    string currentDir = "/some/path/";
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


    pk.keysVec = vector<string>{
            "a", "b", "c", "d", "e", "f", "R", "G", "B", "\n", "\n",
            "%1%100"};

    pk.setOnclick(onClick);
    pk.addKeysByVec(Point(0, 0), Size(45, 45));

    namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    cvui::init(WINDOW_NAME);

    Mat frame, img, mask, img2;

    cap >> pk.bg;
    flipImg(pk.bg, pk.bg);
    int width = 200;
    Mat settingsWin(600, width * 3 + 100, CV_8UC1);
    while (cap.isOpened()) {
        char key = -1;
        cap >> frame;
        flipImg(frame, frame);
        frame.copyTo(img);
        frame.copyTo(img2);
        mask = pk.detectHands(img);
        pk.getClicks();

        pk.draw(img);

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
        if (cvui::button("Change background"))
            key = 'b';
        if (cvui::button("Adjust by QR"))
            key = 'r';
        if (cvui::button("Adjust manually"))
            key = 'm';
        if (cvui::button("Load from file"))
            key = 'l';
        if (cvui::button("Save to file"))
            key = 's';
        if (cvui::button("Adjust scale"))
            key = 'c';
        if (cvui::button("Print"))
            key = 'p';
        cvui::endColumn();

        cvui::update();
        cv::imshow(WINDOW_NAME, settingsWin);

        imshow("img", img);
        imshow("mask", mask);

        if (should_adjustManually)
            pk.adjustKeyboardManually(img2);
        if (should_adjustScale)
            pk.adjustScale(img2);

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
                    if (should_adjustManually)
                        pk.keys.clear();
                    else {
                        destroyWindow(pk.adjKbWName);
                        pk.clearTmpPoints();
                    }
                    if (should_adjustManually && should_adjustScale) {
                        pk.clearTmpPoints();
                        should_adjustScale = false;
                        destroyWindow(pk.adjScWName);
                    }
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
                    if (!should_adjustScale) {
                        pk.clearTmpPoints();
                        destroyWindow(pk.adjScWName);
                    }
                    if (should_adjustScale && should_adjustManually) {
                        pk.clearTmpPoints();
                        should_adjustManually = false;
                        destroyWindow(pk.adjKbWName);
                    }
                    break;
                case 'p': { // print
                    Mat i(A4_SIZE, CV_8UC1, Scalar(255, 255, 255));
                    pk.prepare4Print(i);
                    imshow("Print", i);
                }
                    break;
                default:
                    cout << "Key pressed: " << key << endl;
                    break;
            };
        }

    }
    cap.release();
    return EXIT_SUCCESS;
}