#pragma once
#include "opencv2/opencv.hpp"
#define EPSILON 5e-8

inline void swapMin(int & a, int & b)
{
	if (a > b)
	{
		int t = b;
		b = a;
		a = t;
	}
}
cv::Point3f GetEpipoles(cv::Mat F, cv::Point3f& fe, cv::Point3f& fe2);
cv::Mat CreateCrossMatrix(cv::Point3f x);
int Sign(const int & a);