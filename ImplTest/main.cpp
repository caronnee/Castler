#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

//#define NAME "calibration\\c2\\0062.jpg"
//#define NAME "calibration\\calibrationImages\\Image10.tif"
//#define NAME "calibration\\Canon\\IMG_0254.JPG"
//#define NAME "calibration\\Canon\\IMG_0257.JPG"
#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\mobil\\frames\\chess_8_8\\0017a.jpg"

char * names[] = {
	"calibration\\mobil\\frames\\scaled\\0001.jpg",
	"calibration\\mobil\\frames\\scaled\\0002.jpg",
	"calibration\\mobil\\frames\\scaled\\0003.jpg",
	"calibration\\mobil\\frames\\scaled\\0004.jpg",
	"calibration\\mobil\\frames\\scaled\\0005.jpg",
	"calibration\\mobil\\frames\\scaled\\0006.jpg",
	"calibration\\mobil\\frames\\scaled\\0007.jpg",
	"calibration\\mobil\\frames\\scaled\\0008.jpg",
	"calibration\\mobil\\frames\\scaled\\0009.jpg",
	"calibration\\mobil\\frames\\scaled\\0010.jpg",
	"calibration\\mobil\\frames\\scaled\\0011.jpg",
	"calibration\\mobil\\frames\\scaled\\0012.jpg",
	"calibration\\mobil\\frames\\scaled\\0013.jpg",
	"calibration\\mobil\\frames\\scaled\\0014.jpg",
	"calibration\\mobil\\frames\\scaled\\0015.jpg",
	"calibration\\mobil\\frames\\scaled\\0016.jpg",
	"calibration\\mobil\\frames\\scaled\\0017.jpg",
	"calibration\\mobil\\frames\\scaled\\0017b.jpg",
	"calibration\\mobil\\frames\\scaled\\0018.jpg",
	"calibration\\mobil\\frames\\scaled\\0019.jpg",
	"calibration\\mobil\\frames\\scaled\\0020.jpg",
	"calibration\\mobil\\frames\\scaled\\0021.jpg",
	"calibration\\mobil\\frames\\scaled\\0022.jpg",
	"calibration\\mobil\\frames\\scaled\\0023.jpg",
	"calibration\\mobil\\frames\\scaled\\0024.jpg",
	"calibration\\mobil\\frames\\scaled\\0025.jpg",
	"calibration\\mobil\\frames\\scaled\\0026.jpg",
	"calibration\\mobil\\frames\\scaled\\0027.jpg",
	"calibration\\mobil\\frames\\scaled\\0028.jpg",
	"calibration\\mobil\\frames\\scaled\\0029.jpg",
	"calibration\\mobil\\frames\\scaled\\0030.jpg",
	"calibration\\mobil\\frames\\scaled\\0031.jpg",
	"calibration\\mobil\\frames\\scaled\\0032.jpg",
	"calibration\\mobil\\frames\\scaled\\0033.jpg",
	"calibration\\mobil\\frames\\scaled\\0034.jpg",
	"calibration\\mobil\\frames\\scaled\\0035.jpg",
	"calibration\\mobil\\frames\\scaled\\0036.jpg",
	"calibration\\mobil\\frames\\scaled\\0037.jpg",
	"calibration\\mobil\\frames\\scaled\\0038.jpg",
	"calibration\\mobil\\frames\\scaled\\0039.jpg",
	"calibration\\mobil\\frames\\scaled\\0040.jpg",
	"calibration\\mobil\\frames\\scaled\\0041.jpg",
	"calibration\\mobil\\frames\\scaled\\0042.jpg",
	"calibration\\mobil\\frames\\scaled\\0043.jpg",
	"calibration\\mobil\\frames\\scaled\\0044.jpg",
	"calibration\\mobil\\frames\\scaled\\0045.jpg",
	"calibration\\mobil\\frames\\scaled\\0046.jpg",
	"calibration\\mobil\\frames\\scaled\\0047.jpg",
	"calibration\\mobil\\frames\\scaled\\0048.jpg",
	"calibration\\mobil\\frames\\scaled\\0049.jpg",
	"calibration\\mobil\\frames\\scaled\\0050.jpg",
	"calibration\\mobil\\frames\\scaled\\0051.jpg",
	"calibration\\mobil\\frames\\scaled\\0052.jpg",
	"calibration\\mobil\\frames\\scaled\\0053.jpg",
	"calibration\\mobil\\frames\\scaled\\0054.jpg",
	"calibration\\mobil\\frames\\scaled\\0055.jpg",
	"calibration\\mobil\\frames\\scaled\\0056.jpg",
	"calibration\\mobil\\frames\\scaled\\0057.jpg",
	"calibration\\mobil\\frames\\scaled\\0058.jpg",
	"calibration\\mobil\\frames\\scaled\\0059.jpg",
	"calibration\\mobil\\frames\\scaled\\test.jpg"
	};

int index = 0;

#include <windows.h>

int main()
{
	char bf[1024];
	GetModuleFileNameA(NULL, bf, 1024);
	char * c = strrchr(bf, '\\') + 1;
	(*c) = '\0';
	std::string appPath = bf;

	int total = sizeof(names) / sizeof(names[0]);
	for (int i = 0; i < total; i++)
	{
		////opencv load image
		
		cv::Mat image = cv::imread( (appPath + names[i]).c_str(), CV_LOAD_IMAGE_COLOR);

		//// move to grey
		cv::Mat grey;
		cv::cvtColor(image, grey, CV_BGR2GRAY);
		//// adaptive thresholding
		cv::Mat ret = image;
		cv::Mat resized;
		cv::Size size = image.size();

		std::vector<cv::Point2f> crns;
		bool chess = cv::findChessboardCorners(grey, cv::Size(7, 7), crns, 11);

		if (chess)
			__debugbreak();

		double ratio1 = 800.0 / size.width;
		double ratio2 = 600.0 / size.height;
		if (ratio1 > ratio2)
			ratio1 = ratio2;

		size.width *= ratio1;
		size.height *= ratio1;

		cv::adaptiveThreshold(grey, ret, 255, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 11, 2);

		// find chessboard according to openCV
		std::vector<cv::Point2f> corners;

		// use canny
		cv::Mat blurred;
		/// Reduce noise with a kernel 3x3

		cv::blur(grey, blurred, cv::Size(15, 15));

		cv::Canny(blurred, ret, 20, 60, 3);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(ret, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		ret = image;
		for (int i = 0; i < contours.size(); i++)
		{
			std::vector<cv::Point> & contour = contours[i];
			for (int j = 1; j < contour.size(); j++)
			{
				cv::line(ret, contour[j], contour[j - 1], cv::Scalar(100, 10, 100), 7);
			}
		}
		//bool found = findChessboardCorners(grey, size, corners, 9);
		cv::Mat result;
		cv::resize(ret, result, size);

		//imshow("Result", result);
		// corresponding hugh line

		//cv::waitKey();
	}
	
	return 0;
}