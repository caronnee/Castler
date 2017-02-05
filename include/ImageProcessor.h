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
	cv::Mat _position;
	int imageId;
	bool _estimated;
	ImageInfo()
	{
		_position = cv::Mat::zeros(1, 3, CV_64F);
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
	//checking there was some change to the entry
	bool _modified;

	// hot the phase functions should look like
	typedef bool(ImageProcessor::*Phase) ();//pointer-to-member function 
	
	typedef void(ImageProcessor::*UserInput) (const PointsContext &);//pointer-to-member function 

    //flag is there was some signal o move on
	UserInput _signalAccepted;
	
	// which part of the detecting are we in
	int _phase; 

	// function that should be performed
	Phase _phases[128];

	void InputFeatures( const PointsContext & c);
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
	int _lastIndexSecondary;
	
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
	void FindSecondProjection(cv::Mat essential, int image);

	// extract insistric parameters
	void SplitMatrix(cv::Mat projection, ImageInfo&imageInfo, CameraParams *camera, cv::Mat pos);

	// main global adjustment
	bool GlobalBundleAdjustment();

	// prepares left and right image
	void PrepareDouble(const int& first, const int & second);

	// wait for the input
	bool ManualFeaturesStep();

	// check if some sognal was obtained
	bool InputWait();

	// waits for the input
	bool ManualMatchesStep();
public:

	// class that gives out the results
	IDetector * _detectorOutput;

	// automatically guess calibration 
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
	bool CleanModifiedFlag();
	void ProcessContext(const PointsContext& context);
};