#ifndef PAPERKEYBOARD_QR_H
#define PAPERKEYBOARD_QR_H

#include <opencv2/opencv.hpp>
#include <zbar/zbar.h>
#include <qrencode.h>

using namespace zbar;
using namespace std;
using namespace cv;

typedef struct {
    string type;
    string data;
    vector<Point> location;
    // 0 - left top
    // 1 - left bottom
    // 2 - right bottom
    // 3 - right top
} Decoded_QRCode;

void encodeQr(Mat &img, string text);

void decodeQr(Mat img, vector<Decoded_QRCode> &decodedObjects);

#endif //PAPERKEYBOARD_QR_H
