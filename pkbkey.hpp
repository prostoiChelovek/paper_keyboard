#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


class PKBKey
{
 public:
	Point x1 = Point(-1, -1);
	Point x2, y1, y2;
	string text;

	PKBKey() {}

	PKBKey(Point x1_, Point x2_, Point y1_, Point y2_, string text_)
	{
		x1 = x1_;
		x2 = x2_;
		y1 = y1_;
		y2 = y2_;
		text = text_;
	}

	void draw(Mat &img, Scalar color = Scalar(255, 0, 0), 
               Size fontSize = Size(10, 10))
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