#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

//#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\c2\\0062.jpg"
#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\calibrationImages\\Image10.tif"

int main()
{
	//opencv load image
	cv::Mat image = cv::imread(NAME, CV_LOAD_IMAGE_COLOR);

	// move to grey
	cv::Mat grey;
	cv::cvtColor(image, grey, CV_BGR2GRAY);
	// adaptive thresholding
	cv::Mat threasholded;
	cv::adaptiveThreshold(grey, threasholded, 255, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C,11, 2);
	cv::Mat resized;
	cv::resize(threasholded, resized, cv::Size(300, 500));
	imshow("testing", resized);

	cv::waitKey();
	return 0;
}