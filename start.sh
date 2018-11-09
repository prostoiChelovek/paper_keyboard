#!/usr/bin/env bash
g++ \
    -Wall -Wno-sign-compare -std=c++11 \
    `pkg-config --libs --cflags opencv` `pkg-config --libs --cflags libserial` \
    -I/usr/local/include -lpthread -g /usr/local/lib/libhandDetector.so \
    -L/usr/local/include/handDetector \
    -L/usr/local/include/qrCodeGenerator -g /usr/local/lib/libqrcodegen.a \
    -L/usr/local/include/zbar -lzbar \
    -o main QR.cpp PKBKey.cpp PaperKeyboard.cpp main.cpp
