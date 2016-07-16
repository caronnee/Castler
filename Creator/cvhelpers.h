#pragma once
#include <opencv2/features2d/features2d.hpp>

void KeypointsToPoints(std::vector<cv::KeyPoint>& keypoints,std::vector<cv::Point2f>& points)
{
	for ( int i =0; i < keypoints.size(); i++)
	{
		points.push_back(keypoints[i].pt);
	}
}