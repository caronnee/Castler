#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

typedef std::vector<cv::Point2f> CoordsArray;
typedef std::vector<std::vector<cv::Point2f>> CoordsArray2;
typedef std::vector<cv::Point3f> PointsArray;
typedef std::vector<std::vector<cv::Point3f>> PointsArray2;
typedef std::vector<int> PolyIndices;
