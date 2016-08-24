#pragma once

#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "imageprovider.h"
#include <set>

class ImageProcessor
{
	cv::Scalar _colors;
	cv::FastFeatureDetector *  _ffd;
	std::vector<cv::KeyPoint> _keypoints;
	cv::Mat _frame, _gr;
	std::vector<cv::Point2f> _currentPoints;
	IImageProvider * _provider;
	cv::Mat _ret;
	cv::Size _frameSize;
	// handler for the messages
	IReportFunction * _reporter;
public:
	bool Next();
	ImageProcessor(IReportFunction * reporter, IImageProvider * provider);
	bool Init(bool ffd = false);
	cv::Size GetSize();
	bool Prepare(cv::Mat& frame, std::vector<cv::KeyPoint>& corners, cv::Mat& gr, std::vector<cv::Point2f>&points);
	bool PerformCalibration(int chessWidth, int chessHeight, std::vector<cv::Point2f>& corners);
	bool PerformDetection();
	void UseGrey();
	bool DrawFeatures();
	void ToGrey();
	cv::Mat GetResult();
};