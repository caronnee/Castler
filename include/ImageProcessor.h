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

struct RelMap
{
	double rel;
	int image1;
	int image2;
};

struct CameraParams
{
	// insistric parameters
	cv::Mat insistric;
	// distortion coefficients
	cv::Mat distCoefs;
	// check if this was already estimated
	bool calibrated;
	// name of th camera
	const char * name;
};

struct ImageInfo
{
	CameraParams * _camera;
	cv::Mat _extrinsic;
	cv::Point3f _position;
	int imageId;
	bool _estimated;
	ImageInfo()
	{
		_position = cv::Point3f(0, 0, 0);
		_estimated = false;
		_extrinsic = cv::Mat::eye(3, 3, CV_64F);
	}
};


class IDetector
{
public:
	//virtual void CameraCallback(cv::Point3f, int index) = 0;
	virtual void PointCallback(cv::Point3f & point, int id) = 0;
	~IDetector() {}
};

class ImageProcessor
{
	typedef bool(ImageProcessor::*Phase) ();//pointer-to-member function 
	
	// which part of the detecting are we in
	int _phase; 

	// function that should be performed
	Phase _phases[128];

	// checkpoints to detect for calibration purposes
	std::vector<cv::Point3f> _chesspoints;

	// last index can be basically anything. Just context
	int _lastIndex;
private:
	// found cameras in calibration
	std::vector<CameraParams *> _cameras;

	// color to be used in wire frame
	cv::Scalar _colors;

	// feature detected
	cv::Ptr<cv::Feature2D>  _ffd;

	// vector of how much are the shot reliable
	std::vector<RelMap> _reliability;

	// last index processed
	int _lastImageIndex;
	
	// coordinates, [imageId, pointId] coord. imageId is based on the provider
	KeypointsArray2 _foundCoords;

	// found descriptors
	MatArray _foundDesc;

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

	int FindNextBestPair(int& firstCamera, int & secondCamera);

	// create matches 
	void PrepareCalibration();

	// return projection from essential matrix
	cv::Mat FindSecondProjection(cv::Mat essential, int image);

	// extract insistric parameters
	void SplitMatrix(cv::Mat projection, ImageInfo&imageInfo, CameraParams *camera);

	bool GlobalBundleAdjustment();
public:

	// class that gives out the results
	IDetector * _detectorOutput;

	bool AutoCalibrate();

	// finishes calibration according to the known object
	bool FinishCalibration(PointsArray & chesses, cv::Mat& cameraMatrix, cv::Mat& distCoeffs);

	bool FinishCalibration();

	// from all detected structure it hopefully creates mesh
	bool FinishCreationStep();

	// handler for the messages
	static IReportFunction * _reporter;

	// construct
	ImageProcessor();

	bool MatchesStep();
	bool FeaturesStep();
	// set position to the bext image
	bool Next();
	bool CalibrateStep();
	bool CreateChessboards();
	bool Init(Providers * provider, bool ffd = false);
	cv::Size GetSize();
	bool PrepareImage();
	bool PerformCalibration(int chessWidth, int chessHeight, std::vector<cv::Point2f>& corners);
	void ApplyFeatureDetector();
	bool PerformDetection();
	void UseGrey();
	bool DrawFeatures();
	cv::Mat GetResult();
	//
	void ApplyCalibration(CalibrationSet & calibrationSet);

	~ImageProcessor();
	void Create(const int& mode);
	void Clear();
};