g++ -Wall `pkg-config --libs --cflags opencv` `pkg-config --libs --cflags libserial` -I/usr/local/include -lpthread -std=c++11 -o main main.cpp && \
./main
