#include "Misc.h"
#include "opencv2/opencv.hpp"
#include <windows.h>

int ArrayContains(int & value, PolyIndices & indices)
{
	for (int i = 0; i < indices.size(); i++)
	{
		if (value == indices[i])
			return i;
	}
	return -1;
}
// surrounding of 5 pixels. Should be enough
int ArrayContains(cv::Point2f & testPoint, CoordsArray & points, float scaleAdjust)
{
	float dist = 25 * scaleAdjust;
	int candidate = -1;
	for (int i = 0; i < points.size(); i++)
	{
		cv::Point2f pnt = points[i] - testPoint;
		float testD = pnt.dot(pnt);
		if (testD < dist)
		{
			dist = testD;
			candidate = i;
		}
	}
	return candidate;
}
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

void SavePoint(FILE * file, cv::Point3f&point)
{
	fprintf(file, "%f %f %f\n", point.x, point.y, point.z);
}

void LoadPoint(FILE * file, cv::Point3f&point)
{
	float x, y, z;
	fscanf(file, "%f %f %f\n", &x, &y, &z);
	point = cv::Point3f(x, y, z);
}

void SaveArrayint(FILE * file, std::vector<int> &point)
{
	fprintf(file, "%d ", (int)point.size());
	for (int i = 0; i < point.size(); i++)
	{
		fprintf(file, "%d ", point[i]);
	}
}

void LoadArrayint(FILE * file, std::vector<int> &point)
{
	int sz;
	fscanf(file, "%d ", &sz);
	point.resize(sz);
	for (int i = 0; i < point.size(); i++)
	{
		fscanf(file, "%d ", &point[i]);
	}
}
void SaveArrayKeypoint(FILE * file, std::vector<cv::KeyPoint> &points)
{
	size_t s = points.size();
	fwrite( &s,sizeof(size_t),1,file);
	fwrite(points.data(), sizeof(cv::KeyPoint), s, file);
}

void LoadArrayKeypoint(FILE * file, std::vector<cv::KeyPoint> &points)
{
	size_t s;
	int count = fread(&s, sizeof(size_t), 1, file);
	if (count == 0)
		return;
	points.resize(s);
	fread(points.data(), sizeof(cv::KeyPoint), s, file);
}