#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

//#define NAME "c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\c2\\0062.jpg"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\calibrationImages\\Image10.tif"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\Canon\\IMG_0254.JPG"
//#define NAME "c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\Canon\\IMG_0257.JPG"
#define NAME "c:\\work\\DP1\\Castler\\Creator\\calibration\\mobil\\frames\\chess_8_8\\0017a.jpg"

char * names[] = {
	"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0001.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0002.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0003.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0004.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0005.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0006.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0007.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0008.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0009.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0010.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0011.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0012.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0013.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0014.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0015.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0016.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0017.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0017b.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0018.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0019.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0020.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0021.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0022.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0023.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0024.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0025.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0026.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0027.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0028.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0029.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0030.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0031.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0032.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0033.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0034.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0035.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0036.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0037.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0038.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0039.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0040.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0041.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0042.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0043.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0044.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0045.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0046.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0047.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0048.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0049.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0050.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0051.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0052.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0053.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0054.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0055.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0056.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0057.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0058.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\0059.jpg"
	,"c:\\work\\DP1\\Castler\\Creator\\bin\\calibration\\mobil\\frames\\chess_8_8\\test.jpg"
};

int index = 0;

int main()
{
	for (int i = 0; i < sizeof(names); i++)
	{
		////opencv load image
		cv::Mat image = cv::imread(names[i], CV_LOAD_IMAGE_COLOR);

		//// move to grey
		cv::Mat grey;
		cv::cvtColor(image, grey, CV_BGR2GRAY);
		//// adaptive thresholding
		cv::Mat ret = image;
		cv::Mat resized;
		cv::Size size = image.size();

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

		imshow("Result", result);
		// corresponding hugh line

		cv::waitKey();
	}
	
	return 0;
}