#include <opencv2/opencv.hpp>
#include <iostream>
#include "vector"
#include <stdlib.h>

using namespace std;
using namespace cv;

VideoCapture cap(0);
double width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
double height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
Mat frame, mask, maskMorph, imgHSV, detectedFacesimg;

Scalar lower = CvScalar(106, 0, 0); // hsv ranges of color filter
Scalar upper = CvScalar(256,256,256);

int minArea = 40*40;
int maxArea = 800*800;

Size keySize(50, 50);

vector<vector<string> > keys;

string text;

Mat mask_morph(Mat mask){
   Mat maskMorph;
   Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
   erode(mask,maskMorph,erodeElement);
   erode(mask,maskMorph,erodeElement);
   dilate(mask,maskMorph,dilateElement);
   dilate(mask,maskMorph,dilateElement);
   return maskMorph;
}

void drawKey(Mat &img, Point pos, string sign){
	pos.x*=keySize.width;
	pos.y*=keySize.height;
	rectangle(img, Rect(pos.x, pos.y, keySize.width, keySize.height), Scalar(204, 51, 0));
	putText(img, String(sign), Point(pos.x + keySize.width/2, pos.y + keySize.height/2), FONT_HERSHEY_DUPLEX, 1, Scalar(204,51,0), 2);
}

void drawKeyboard(Mat &img){
	for(int x = 0; x < keys.size(); x++){
		for(int y = 0; y < keys[x].size(); y++){
			if(keys[x][y].length() != 0)
				drawKey(img, Point(x, y), keys[x][y]);
		}
	}
}

void setKey(string key, Point pos){
	if(keys.size() <= pos.x)
		keys.resize(pos.x+1);
	if(keys[pos.x].size() <= pos.y)
		keys[pos.x].resize(pos.y+1);
	keys[pos.x][pos.y] = key;
}

void pressKey(string key){
	if(key == "<-")
		text = text.substr(0, text.size()-1);
	else
		text += key;
}

void fillKeyboard(Point start){
	vector<string>letters{
		"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "7", "8", "9", "\n",
		"a", "s", "d", "f", "g", "h", "j", "k", "l", "4", "5", "6", "\n",
		"z", "x", "c", "v", "b", "n", "m", "1", "2", "3", "\n",
		" ", "<-", " "
	};
	int x = 0;
	int y = 0;
	for(string& l : letters){
		if(l == "\n"){
			x=0;
			y++;
			continue;
		}
		setKey(l, Point(start.x + x, start.y + y));
		x++;
	}
}

int main(int argc, char **argv){
   Mat blur;
	Point lHigherPoint(0,0);
	Rect Lbr;
	Rect br;
	Point lPtFar;
	Point ptStart, ptEnd, ptFar;
	fillKeyboard(Point(0, 0));
   while(true){
		Point higherPoint(width, height);   
		cap >> frame;
		flip(frame, frame, 0);
		flip(frame, frame, 1);
      frame.copyTo(img);
		drawKeyboard(img);
      GaussianBlur(frame, blur, Size(35, 35), 0, 0);
      medianBlur(blur, blur, 11);
      cvtColor(blur, imgHSV, COLOR_BGR2HSV);
      inRange(imgHSV, lower, upper, mask);
      maskMorph = mask_morph(mask);
      vector< vector<Point> > contours;
      vector<Vec4i> hierarchy;
      findContours(maskMorph,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
      vector<vector<Point> >hull(contours.size());   
      vector<vector<int> > hullI(contours.size());
      vector<vector<Vec4i> > defects(contours.size());
      if (hierarchy.size() > 0) {
         for (int i = 0; i >= 0; i = hierarchy[i][0]) {
               Moments moment = moments((cv::Mat)contours[i]);
               double area = moment.m00;
					if(area>minArea&&area<maxArea){
                  convexHull(contours[i], hull[i], false);
                  convexHull(contours[i], hullI[i], false);
                  if(hullI[i].size() < 3)
                     continue;
                  convexityDefects(contours[i], hullI[i], defects[i]);
                  for(const Vec4i& v : defects[i]){
                        float depth = v[3] / 256;
                        if (depth < 11)
									continue;									
                        int startidx = v[0];ptStart = Point(contours[i][startidx]);
                        int endidx = v[1];ptEnd = Point(contours[i][endidx]);
                        int faridx = v[2];ptFar = Point(contours[i][faridx]);
								line(img, ptStart, ptEnd, Scalar(0, 255, 0), 2);
                        line(img, ptStart, ptFar, Scalar(0, 255, 0), 2);
                     	line(img, ptEnd, ptFar, Scalar(0, 255, 0), 3); 
						      circle(img, ptFar, 5, Scalar(255, 255, 255), 2);
								circle(img, ptEnd, 10, Scalar(0, 0, 0), 2);
								if(ptEnd.y < higherPoint.y){
									higherPoint = ptEnd;
								}
						}
						br = boundingRect(contours[i]);
						rectangle(img, br, Scalar(0, 255, 255));
						circle(img, higherPoint, 10, Scalar(0, 0, 255), 2);
						//br.height + 5 <= Lbr.height
						//cout << "ptStart: " << ptStart.x << ":" << ptStart.y << endl;
						//cout << "ptEnd: " << ptEnd.x << ":" << ptEnd.y << endl;
						//cout << "ptFar: " << ptFar.x << ":" << ptFar.y << endl;
						if(higherPoint.y - ptFar.y - 10 >= lHigherPoint.y - lPtFar.y && higherPoint.x != width 
														 && higherPoint.y != height){
							int btnX = higherPoint.x / keySize.width;
							int btnY = higherPoint.y / keySize.height;
							if(keys.size() > btnX){
								if(keys[btnX].size() > btnY){
									if(keys[btnX][btnY].length() != 0){
										cout <<  "click: " << higherPoint.x << ":" << higherPoint.y << " " << btnX << ":" << btnY << endl;						
										cout << keys[btnX][btnY] << endl;
										pressKey(keys[btnX][btnY]);
									}
								}
							}
						}
						drawContours(img, contours, i, Scalar(255, 255, 0, 0), 2, 8, hierarchy, 0, Point(1, 1));
				}
         }
			Lbr = br;
			lHigherPoint = higherPoint;
			lPtFar = ptFar;
      }
		putText(img, String(text), Point(50, 400), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255,0), 2);
      imshow("mask", mask);
      imshow("img", img);
      
     cvWaitKey(1);
   }
}