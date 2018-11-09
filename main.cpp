#include <opencv2/opencv.hpp>

#include "PaperKeyboard.h"

#include <SerialStream.h>

using namespace std;
using namespace cv;
using namespace LibSerial;

SerialStream serial;

bool openSerial(string port)
{
    serial.Open(port);
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

void onClick(const Point &p, const PKBKey &k)
{
    cout << k.text << endl;
    writeSerial(k.text);
}

int main(int argc, char **argv)
{
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Unable to open video capture" << endl;
        return EXIT_FAILURE;
    }

    string portDev = "/dev/ttyACM0";
    if (!openSerial(portDev)) {
        cerr << "Unable to open serial port " << portDev << endl;
    } else {
        cout << "Serial port " << portDev << " opened successfully" << endl;
    }

    PaperKeyboard pk;

    /*pk.keysVec = vector<string>{
         "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
         "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
         "z", "x", "c", "v", "b", "n", "m", "\n",
         "space", "bkspace"};
    pk.keysVec = vector<string>{
         "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
         "й", "ц", "у", "к", "е", "н", "г", "ш", "щ", "з", "х", "ъ",
         "ф", "ы", "в", "а", "п", "р", "о", "л", "д", "ж", "э",
         "я", "ч", "с", "м", "и", "т", "ь", "б", "ю",
    };*/

    pk.keysVec = vector<string>{
            "1", "2", "3", "4", "5", "6", "R", "G", "B"};

    pk.setOnclick(onClick);
    pk.addKeysByVec(Point(50, 200), Size(30, 30));

    Mat frame, img, mask, img2;

    Mat i(A4_SIZE, CV_8UC1, Scalar(255, 255, 255));
    pk.prepare4Print(i);

    pk.adjustKeyboardByQR(i);

    cap >> pk.bg;
    flipImg(pk.bg, pk.bg);
    pk.adjustColorRanges();
    while (cap.isOpened()) {
        cap >> frame;
        flipImg(frame, frame);
        frame.copyTo(img);
        frame.copyTo(img2);

//        pk.adjustKeyboard(img2);
        mask = pk.detectHands(img);
        pk.getClicks();

        pk.draw(img);
        imshow("img", img);
        imshow("mask", mask);

        pk.setLast();

        // handling keystrokes
        char key = waitKey(1);
        if (key != -1) {
            if (key == 'q' || key == 'Q') { // exit
                cout << "exit" << endl;
                return EXIT_SUCCESS;
            } else if (key == 'b' || key == 'B') { // change bg
                cap >> pk.bg;
                flipImg(pk.bg, pk.bg);
                cout << "Background changed." << endl;
            } else {
                printf("Key pressed: %c\n", key);
            }
        }

    }
    cap.release();
    return EXIT_SUCCESS;
}