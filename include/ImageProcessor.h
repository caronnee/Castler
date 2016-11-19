#pragma once

#include "typedefs.h"
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "imageprovider.h"
#include <set>

struct MatchPair
{
	int image1, image2, index1, index2;
};

class ImageProcessor
{
	cv::Scalar _colors;
	cv::Ptr<cv::Feature2D>  _ffd;

	int _lastImageIndex;
	// coordinates, [imageId, pointId] coord. imageId is based on the provider
	KeypointsArray2 _foundCoords;

	// partial matches
	std::vector<MatchPair> _matches;

	// final cloud point
	Points3D _meshPoints;

	// current frame;
	cv::Mat _frame, _gr;

	// provider for the frames
	Providers * _provider;
	
	// current return image
	cv::Mat _ret;

	// current framesize
	cv::Size _frameSize;
	
public:

	// handler for the messages
	static IReportFunction * _reporter;
	bool Next();
	ImageProcessor();
	bool Init( Providers * provider, bool ffd = false);
	cv::Size GetSize();
	bool PerformCalibration(int chessWidth, int chessHeight, std::vector<cv::Point2f>& corners);
	void ApplyFeatureDetector();
	bool PerformDetection();
	void UseGrey();
	bool DrawFeatures();
	cv::Mat GetResult();
	void ApplyCalibration(CalibrationSet & calibrationSet);
	bool FinishCalibration(PointsArray & chesses, cv::Mat& cameraMatrix, cv::Mat& distCoeffs);
};