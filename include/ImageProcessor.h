#pragma once

#include "typedefs.h"
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "imageprovider.h"
#include <set>

struct MatchPair
{
	int imageId;
	int pointdId;
	// next that was not paired yet to anyone
	MatchPair * paired;
	// this correspond to the finally reconstructed point
	int id = -1;
};

struct CameraParams
{
	cv::Mat insistric;
	bool calibrated;
};

struct ImageInfo
{
	CameraParams * _camera;
	cv::Mat _extrinsic;
	int imageId;
	ImageInfo()
	{
		_extrinsic = cv::Mat::eye(3, 3, CV_64F);
	}
};

typedef void (*PointCallback)(cv::Point3f, int index);

class ImageProcessor
{
public:

	// point call to to something when point was reconstructes
	PointCallback _pointCallback = NULL;

	// TODO change to camera calback
	PointCallback _cameraAdjustedCallback = NULL;

private:

	// found cameras in calibration
	std::vector<CameraParams> _cameras;

	// color to be used in wire frame
	cv::Scalar _colors;

	// feature detected
	cv::Ptr<cv::Feature2D>  _ffd;

	// vector of how much are the shot reliable
	DoubleVector _reliability;

	// last index processed
	int _lastImageIndex;
	
	// coordinates, [imageId, pointId] coord. imageId is based on the provider
	KeypointsArray2 _foundCoords;

	// found descriptors
	MatsArray2 _foundDesc;

	// partial matches
	std::vector<MatchPair *> _matches;

	// detected information about images. keypoints are save separatelly
	std::vector<ImageInfo> _imagesInfo;

	// final cloud point
	Points3D _meshPoints;

	// current frame;
	cv::Mat _frame, _gr;

	// provider for the frames
	Providers * _provider;
	
	// current return image
	cv::Mat _ret;

	// current frame size associated with the latest Next call
	cv::Size _frameSize;

	int FindBestPair(int& firstCamera, int & secondCamera);

	// create matches 
	void PrepareCalibration();

	// return projection from essental matrix
	cv::Mat FindProjection(cv::Mat essential, int image);

	// extract insinstric parameters
	void SplitMatrix(cv::Mat projection, cv::Mat& extrinsic, CameraParams *camera);

public:

	// finishes calibration according to the known object
	bool FinishCalibration(PointsArray & chesses, cv::Mat& cameraMatrix, cv::Mat& distCoeffs);

	// create matches between images
	void CreateMatches();

	// from all detected structure it hopefully creates mesh
	void FinishCreation();

	// handler for the messages
	static IReportFunction * _reporter;

	// construct
	ImageProcessor();

	// set position to the bext image
	bool Next();
	bool Init( Providers * provider, bool ffd = false);
	cv::Size GetSize();
	bool PerformCalibration(int chessWidth, int chessHeight, std::vector<cv::Point2f>& corners);
	void ApplyFeatureDetector();
	bool PerformDetection();
	void UseGrey();
	bool DrawFeatures();
	cv::Mat GetResult();
	//
	void ApplyCalibration(CalibrationSet & calibrationSet);

};