g++ -Wall `pkg-config --libs --cflags opencv` `pkg-config --libs --cflags libserial` -I/usr/local/include -lpthread -std=c++11 -g /usr/local/lib/libhandDetector.so -L/usr/local/include/handDetector -o main PKBKey.cpp PaperKeyboard.cpp main.cpp && \
./main
