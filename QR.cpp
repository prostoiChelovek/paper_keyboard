#include "QR.h"

// https://www.learnopencv.com/barcode-and-qr-code-scanner-using-zbar-and-opencv/
void decodeQr(const Mat img, vector<Decoded_QRCode> &decodedObjects) {
    // Create zbar scanner
    ImageScanner scanner;

    // Configure scanner
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    // Convert image to grayscale
    Mat imGray;
    if (img.channels() != 1)
        cvtColor(img, imGray, COLOR_BGR2GRAY);
    else
        img.copyTo(imGray);

    // Wrap image data in a zbar image
    Image image(img.cols, img.rows, "Y800", imGray.data, img.cols * img.rows);

    // Scan the image for barcodes and QRCodes
    scanner.scan(image);

    // Print results
    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        Decoded_QRCode obj;

        obj.type = symbol->get_type_name();
        if (obj.type != "QR-Code")
            continue;

        obj.data = symbol->get_data();

        // Obtain location
        for (int i = 0; i < symbol->get_location_size(); i++) {
            obj.location.emplace_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
        }

        decodedObjects.push_back(obj);
    }
}

// img must have size
void encodeQr(Mat &img, string text) {
    int version = 4;
    QRecLevel level = QR_ECLEVEL_M;
    QRencodeMode hint = QR_MODE_8;
    int casesensitive = 1;
    int size = img.cols;
    int margin = 2;

    QRcode *code = QRcode_encodeString(text.c_str(), version, level, hint, casesensitive);
    if (code == nullptr) {
        cerr << "QR code generation error" << endl;
        return;
    }

    int code_size = size / code->width;
    code_size = (code_size == 0) ? 1 : code_size;
    int img_width = code->width * code_size + 2 * margin;
    if (img.cols != img.rows)
        resize(img, img, Size(img_width, img_width));
    u_char *p = code->data;
    int x, y, posx, posy;
    for (y = 0; y < code->width; y++) {
        for (x = 0; x < code->width; x++) {
            if (*p & 1) {
                posx = x * code_size + margin;
                posy = y * code_size + margin;
                rectangle(img, Rect(posx, posy, code_size, code_size),
                          Scalar(0, 0, 0), CV_FILLED);
            }
            p++;
        }
    }
}