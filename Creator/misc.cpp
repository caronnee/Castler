#include "Misc.h"
#include "opencv2/opencv.hpp"
#include <windows.h>

cv::Point3f GetEpipoles(cv::Mat F, cv::Point3f& fe,cv::Point3f& fe2)
{
	cv::Mat u, v, w;
	cv::SVD::compute(F, w, u, v, cv::SVD::FULL_UV);

	// svd is sorted in decreasing order. Last one must be 0
	int lastCol = w.cols -1;
	fe.x = v.at<double>(0, lastCol);
	fe.y = v.at<double>(1, lastCol);
	fe.z = v.at<double>(2, lastCol);

	fe2.x = v.at<double>(lastCol,0 );
	fe2.y = v.at<double>(lastCol,1 );
	fe2.z = v.at<double>(lastCol,2 );

	return fe;
}

cv::Mat CreateCrossMatrix(cv::Point3f x)
{
	cv::Mat A = cv::Mat::zeros(3, 3,CV_64F);
	const double a1 = x.x, a2 = x.y, a3 = x.z;

	A.at<double>(0, 0) = 0;
	A.at<double>( 0, 1) = -a3;
	A.at<double>( 0, 2) = a2;

	A.at<double>( 1, 0) = a3;
	A.at<double>( 1, 1) = 0;
	A.at<double>( 1, 2) = -a1;

	A.at<double>( 2, 0) = -a2;
	A.at<double>( 2, 1) = a1;
	A.at<double>( 2, 2) = 0;

	return A;
}

int Sign(const int & a)
{

	if (a < -EPSILON)
		return -1;
	if (a > EPSILON)
		return 1;
	return 0;
}
