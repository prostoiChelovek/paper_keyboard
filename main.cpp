#include <opencv2/opencv.hpp>
#include <iostream>

#include "PaperKeyboard.hpp"

#include <SerialStream.h>
#include <SerialPort.h>

#define Q_KEY 1048689
#define B_KEY 1048674

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
	if (!serial.good())
	{
		return false;
	}
	return true;
}

bool writeSerial(string str)
{
	if (serial.good())
	{
		usleep(5000);
	}
	else
	{
		return false;
	}
	serial << str;
	usleep(100000);
	return true;
}

void flipImg(Mat img, Mat &res)
{
	flip(img, res, 0);
	flip(img, res, 1);
}

void onClick(const Point &p, const PKBKey &k)
{
	writeSerial(k.text);
}

int main(int argc, char **argv)
{
	VideoCapture cap(1);
	if (!cap.isOpened())
	{
		cerr << "Unable to open video capture" << endl;
		return EXIT_FAILURE;
	}

	string portDev = "/dev/ttyACM0";
	if (!openSerial(portDev))
	{
		cerr << "Unable to open serial port " << portDev << endl;
	}
	else
	{
		cout << "Serial port " << portDev << " opened successfully" << endl;
	}

	PaperKeyboard pk;

	pk.onClickCallback = onClick;

	//pk.addKey(Point(5, 5), Point(65, 5), Point(7, 60), Point(55, 55), "a");
	//pk.addKeysByVec(Point(10, 10), keys, 50, 50);

	Mat frame, img, mask, img2;

	cap >> pk.bg;
	flipImg(pk.bg, pk.bg);
	pk.adjustColorRanges();
	while (cap.isOpened())
	{
		cap >> frame;
		flipImg(frame, frame);
		frame.copyTo(img);
		frame.copyTo(img2);

		pk.checkPrintedText();
		pk.adjustKeyboard(img2);
		mask = pk.detectHands(img);
		pk.getClicks();

		pk.draw(img);
		imshow("img", img);
		imshow("mask", mask);

		pk.setLast();

		// handling keystrokes
		int key = waitKeyEx(1);
		if (key != -1)
			printf("Key presed: %i\n", key);
		switch (key)
		{
		case Q_KEY: // exit
			return EXIT_SUCCESS;
		case B_KEY: // change bg
			cap >> pk.bg;
			flipImg(pk.bg, pk.bg);

			cout << "Background changed." << endl;
			break;
		}
	}
	cap.release();
	return EXIT_SUCCESS;
}