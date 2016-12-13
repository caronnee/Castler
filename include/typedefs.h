#pragma once

#include <vector>
#include <map>
#include <opencv2/opencv.hpp>


typedef std::vector<cv::Mat> MatArray;
typedef std::vector<MatArray> MatsArray2;
typedef std::vector<cv::Point3f> Points3D;
typedef std::vector<cv::Point2f> CoordsArray;
typedef std::vector<cv::KeyPoint> KeypointsArray;
typedef std::vector<KeypointsArray> KeypointsArray2;
typedef std::vector<std::vector<cv::Point2f>> CoordsArray2;
typedef std::vector<cv::Point3f> PointsArray;
typedef std::vector<std::vector<cv::Point3f>> PointsArray2;
typedef std::vector<int> PolyIndices;
typedef std::map<std::pair<int,int>, int[2] > PairMap;
typedef std::pair<int, int> IntPair;
typedef std::vector<double> DoubleVector;