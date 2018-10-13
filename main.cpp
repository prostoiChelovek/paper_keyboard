#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>

#include "handDetector.hpp"

#include <SerialStream.h>
#include <SerialPort.h>

#define Q_KEY 1048689
#define B_KEY 1048674

#define fontSize Size(10, 10)

using namespace std;
using namespace cv;
using namespace LibSerial;

class Key
{
 public:
	Point x1 = Point(-1, -1);
	Point x2, y1, y2;
	string text;

	Key() {}

	Key(Point x1_, Point x2_, Point y1_, Point y2_, string text_)
	{
		x1 = x1_;
		x2 = x2_;
		y1 = y1_;
		y2 = y2_;
		text = text_;
	}

	void draw(Mat &img, Scalar color = Scalar(255, 0, 0))
	{
		line(img, x1, x2, color);
		line(img, y1, y2, color);
		line(img, x1, y1, color);
		line(img, x2, y2, color);
		putText(img, text,
				  Point((x1.x + x2.x) / 2 - fontSize.width * (text.size()),
						  (x1.y + y1.y) / 2 + fontSize.height),
				  FONT_HERSHEY_COMPLEX, 1, color);
	}
};

class PaperKeyboard
{
 public:
	Mat bg;

	HandDetector hd;
	Scalar YCrCb_lower = Scalar(0, 135, 90);
	Scalar YCrCb_upper = Scalar(200, 209, 150);

	vector<Key> keys;
	Finger lastHigherFinger;
	string printedText = "";
	function<void(const Point &, const Key &)> onClickCallback;

	time_t lastClickTime = time(0);

	/*vector<string> keyboardKeysVec{
		 "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
		 "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
		 "z", "x", "c", "v", "b", "n", "m", "\n",
		 "space", "bkspace"};*/
	vector<string> keyboardKeysVec{
		 "1", "2", "3", "4", "5", "6", "R", "G", "B"};
	/*vector<string> keyboardKeysVec{
		 "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
		 "й", "ц", "у", "к", "е", "н", "г", "ш", "щ", "з", "х", "ъ", 
		 "ф", "ы", "в", "а", "п", "р", "о", "л", "д", "ж", "э",
		 "я", "ч", "с", "м", "и", "т", "ь", "б", "ю",  
	};*/
	PaperKeyboard()
	{
		hd = HandDetector();
	}

	Mat detectHands(Mat img)
	{
		Mat imgYCrCb, mask;
		hd.deleteBg(img, bg, img);

		cvtColor(img, imgYCrCb, COLOR_BGR2YCrCb);
		mask = hd.detectHands_range(imgYCrCb, YCrCb_lower, YCrCb_upper);

		hd.getFingers();
		hd.getCenters();
		hd.getHigherFingers();

		return mask;
	}

	// TODO: fix this
	void setLast()
	{
		if (hd.hands.size() > 0)
		{
			if (hd.hands[0].higherFinger.ok)
				lastHigherFinger = hd.hands[0].higherFinger;
		}
	}

	void adjustColorRanges()
	{
		String wName = "adjust color ranges";
		namedWindow(wName);
		auto onTrackbarActivity = [](int val, void *data) {
			double &vNum = *(static_cast<double *>(data));
			vNum = double(val);
		};
		createTrackbar("CrMin", wName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[0]);
		createTrackbar("CrMax", wName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[0]);
		createTrackbar("CbMin", wName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[1]);
		createTrackbar("CbMax", wName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[1]);
		createTrackbar("YMin", wName, 0, 255, onTrackbarActivity, &YCrCb_lower.val[2]);
		createTrackbar("YMax", wName, 0, 255, onTrackbarActivity, &YCrCb_upper.val[2]);
		setTrackbarPos("CrMin", wName, int(YCrCb_lower.val[0]));
		setTrackbarPos("CrMax", wName, int(YCrCb_upper.val[0]));
		setTrackbarPos("CbMin", wName, int(YCrCb_lower.val[1]));
		setTrackbarPos("CbMax", wName, int(YCrCb_upper.val[1]));
		setTrackbarPos("YMin", wName, int(YCrCb_lower.val[2]));
		setTrackbarPos("YMax", wName, int(YCrCb_upper.val[2]));
	}

	vector<Point> tmpPoints;

	void adjustKeyboard(Mat &img)
	{
		String wName = "adjust keyboard";
		namedWindow(wName);

		auto mouseCallback = [](int event, int x, int y, int, void *data) {
			vector<Point> &tmpPoints = *(static_cast<vector<Point> *>(data));
			switch (event)
			{
			case CV_EVENT_LBUTTONDOWN:
				tmpPoints.push_back(Point(x, y));
				break;
			};
		};
		setMouseCallback(wName, mouseCallback, &tmpPoints);
		if (tmpPoints.size() == 4)
		{
			addKey(tmpPoints[0], tmpPoints[1], tmpPoints[2], tmpPoints[3],
					 keyboardKeysVec[keys.size()]);
			tmpPoints.clear();
		}

		putText(img, keyboardKeysVec[keys.size()], Point(10,100), 
					FONT_HERSHEY_DUPLEX, 1, Scalar(0,143,143), 2);

		for (Point p : tmpPoints)
		{
			circle(img, p, 3, Scalar(0, 0, 0));
		}
		imshow(wName, img);
	}

	void addKey(Point x1, Point x2, Point y1, Point y2, string text)
	{
		keys.push_back(Key(x1, x2, y1, y2, text));
	}

	void addKeysByVec(Point x1, vector<string> strs, int height = 50,
							int width = 50)
	{
		int i = 1;
		int r = 1;
		int lX = x1.x;
		for (string &s : strs)
		{
			auto fN = s.find("\n");
			if (fN != -1)
			{
				r++;
				i = 1;
				lX = x1.x;
				s.replace(fN, fN + 2, "");
				continue;
			}
			int aW = s.size() > 1 ? fontSize.width * (s.size() + 1) : 0;
			Point X1(lX, x1.y + width * r);
			Point X2(X1.x + width + aW, X1.y);
			addKey(X1, X2, Point(X1.x, X1.y + height),
					 Point(X2.x, X1.y + height), s);
			i++;
			lX = X2.x;
		}
	}

	void deleteKeysByText(string text, int num = 1)
	{
		int n = 0;
		for (int i = 0; i < keys.size(); i++)
		{
			if (n == num)
				break;
			if (keys[i].text == text)
			{
				keys.erase(keys.begin() + i);
				n++;
			}
		}
	}

	Key getKeyByPoint(Point p)
	{
		for (Key k : keys)
		{
			if (k.x1.x < p.x && k.x2.x > p.x &&
				 k.x1.y < p.y && k.y1.y > p.y)
				return k;
		}
		return Key();
	}

	// TODO: make correct click detection
	void getClicks()
	{
		if (hd.hands.size() >= 1)
		{
			Finger &cf = hd.hands[0].higherFinger;
			if (cf.ok && lastHigherFinger.ok)
			{
				int diffDist = (cf.ptFar.y - cf.ptStart.y) - (lastHigherFinger.ptFar.y - lastHigherFinger.ptStart.y);
				if (diffDist > 5 && diffDist < 40)
				{
					Key k = getKeyByPoint(Point(cf.ptStart.x + 10, cf.ptStart.y));
					if (k.x1.x != -1)
					{
						if (time(0) - lastClickTime >= 1){
							onClick(cf.ptStart, k);
							lastClickTime = time(0);
						}
					}
				}
			}
		}
	}

	void onClick(const Point &p, const Key &k)
	{
		cout << "click " << k.text << endl;

		if (k.text == "space")
		{
			printedText += " ";
		}
		else if (k.text == "bkspace")
		{
			printedText = printedText.substr(0, printedText.size() - 1);
		}
		else
		{
			printedText += k.text;
		}
		onClickCallback(p, k);
	}

	void checkPrintedText()
	{
		if(printedText.size() >= 10)
			printedText = "";
	}

	void drawKeys(Mat &img, Scalar color = Scalar(255, 0, 0))
	{
		for (Key &k : keys)
			k.draw(img, color);
	}
	void drawText(Mat &img, Scalar color = Scalar(0, 255, 0), Point pos = Point(200, 200))
	{
		putText(img, printedText, pos, FONT_HERSHEY_DUPLEX, 1, color, 2);
	}
	void draw(Mat &img, Scalar color = Scalar(255, 0, 0))
	{
		drawKeys(img, color);
		drawText(img, color, Point(10, 450));
		hd.drawHands(img, color);
	}
};

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

void onClick(const Point &p, const Key &k)
{
	writeSerial(k.text);
}

int main(int argc, char **argv)
{
	VideoCapture cap(0);
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