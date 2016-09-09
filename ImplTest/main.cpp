#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

//#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\c2\\0062.jpg"
#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\calibrationImages\\Image10.tif"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\Canon\\IMG_0254.JPG"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\Canon\\IMG_0257.JPG"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\mobil\\frames\\chess_8_8\\0017.jpg"

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

	cv::Size size(7, 7);
	std::vector<cv::Point2f> corners;
	bool found = findChessboardCorners(grey, size, corners, 9);
	cv::waitKey();
	return 0;
}