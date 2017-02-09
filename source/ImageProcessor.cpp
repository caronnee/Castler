#include "ImageProcessor.h"
#include "typedefs.h"
#include "Misc.h"

IReportFunction * ImageProcessor::_reporter = NULL;

struct SerializeCallback
{
	SerializeKeyPoints serializeKeypoint;
};

bool ImageProcessor::AutoCalibrate()
{
	_reporter->Report(MInfo, "Initializing camera");
	if (_cameras.empty())
	{
		CameraParams * params = new CameraParams;
		params->insistric = cv::Mat::eye(3, 3, CV_64F);
		params->calibrated = true;
		params->name = "Testing one";
		_cameras.push_back(params);
	}

	for (int i = 0; i < _imagesInfo.size(); i++)
	{
		//TODO
		_imagesInfo[i]._camera = _cameras[0];
	}
	// create tracks from all matches that we discovered
	int points = 0;
	for (int i = 0; i < _matches.size(); i++)
	{
		if (_matches[i]->id == -1)
			_matches[i]->id = points++;
		if (_matches[i]->paired)
			_matches[i]->paired->id = points;
	}
	_meshPoints.clear();;
	_meshPoints.resize(points);
	_lastIndexSecondary = 1;
	_lastIndex = 0;
	// done. Nothing more to do
	return false;
}

// structure to load features
struct StreamFeatures
{
	//keypoint save/load function

};

void ImageProcessor::InputFeatures(const PointsContext & c)
{
	KeypointsArray k;
	cv::KeyPoint::convert(c.p1, k);
	_foundCoords.push_back(k);
	// zero. Just to indicate that we do not have any foundcoords
	_foundDesc.push_back(cv::Mat());
	// save to the files
	FILE* file = fopen((_provider->Name() + ".keys").toStdString().c_str(), "wb");
	if (file)
	{
		std::vector<cv::KeyPoint> points;
		cv::KeyPoint::convert(c.p1,points);
		SaveArrayKeypoint(file, points);
		fclose(file);
	}

	(_provider->Name(),k);
}

int ImageProcessor::FindNextBestPair(int& firstCamera, int & secondCamera)
{
	// and it must have good inline ratio, from the homography calculation
	int bestIndex = 0;
	double best = 0.0;
	int nframes = _foundCoords.size();
	int framesCount = nframes;
	int count = nframes;
	int image1, image2;
	for (int i = 0; i < _reliability.size(); i++)
	{
		count--;
		if (count == 0)
		{
			count = --nframes;
		}
		if (_reliability[i].rel > best)
		{
			if (firstCamera == secondCamera)
			{
				best = _reliability[i].rel;
				image2 = _reliability[i].image2;
				image1 = _reliability[i].image1;
			}
			else
			{
				// one of the images must be already determined
				int t2 = _reliability[i].image2;
				int t1 = _reliability[i].image1;
				if (_imagesInfo[t1]._estimated != _imagesInfo[t2]._estimated)
				{
					best = _reliability[i].rel;
					image2 = t2;
					image1 = t1;
				}
			}
		}
	}
	int ret = (firstCamera == secondCamera) ? 2 : 1;
	secondCamera = image2;
	firstCamera = image1;
	return ret;
}

ImageProcessor::ImageProcessor()
{
	_signalAccepted = NULL;
	_modified = false;
	_signalAccepted = false;
	_lastIndex = 0;
	_phase = 0;
	_detectorOutput = NULL;
	_provider = NULL;
	_colors = cv::Scalar(0, 1, 0);
	_lastIndexSecondary = 1;
}

bool ImageProcessor::FeaturesStep()
{
	if (PrepareImage() == false)
	{
		_provider->Reset();
		_lastIndexSecondary = 1;
		_reporter->Report(MInfo, "All entries processed");
		return false;
	}
	QString message = QString::asprintf("Processing %d input ( %s )", _lastIndexSecondary, _provider->Name().toStdString().c_str());
	_reporter->Report(MInfo, message);
	ApplyFeatureDetector();
	return true;
}

bool ImageProcessor::Init(Providers * provider, bool ffd /*= false*/)
{
	_provider = provider;
	_imagesInfo.resize(_provider->Count());
	// good features to track
	//_ffd = cv::GFTTDetector::create();
	_ffd = cv::ORB::create();
	//_ffd = cv::FastFeatureDetector::create();
	return true;
}

cv::Size ImageProcessor::GetSize()
{
	return _frameSize;
}

bool ImageProcessor::PrepareImage()
{
	_modified = true;
	if (!_provider || _provider->Frame(_frame) == false)
		return false;

	_frameSize = _frame.size();
	cv::cvtColor(_frame, _gr, CV_RGB2GRAY);
	//cv::goodFeaturesToTrack(gr, corners, 100, 0.1, 10, cv::noArray(), 7);
	_frame.copyTo(_ret);
	return true;
}

bool ImageProcessor::Next( )
{
	if (!_phases[_phase])
		return false;
	
	if ((this->*_phases[_phase])() == false)
	{
		_phase++;
	}
	return true;
}

bool ImageProcessor::FinishCalibration()
{
	// last calibration step
	_reporter->Report(MInfo, "Calculating parameters");

	// finish when all images are processed

	cv::Mat cameraMatrix, distCoeffs, rvecs, tvecs;
	cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 1;
	distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	if (FinishCalibration(_chesspoints, cameraMatrix, distCoeffs))
	{
		_reporter->Report(MInfo, "Calibration successful");
		//emit camParametersSignal(cameraMatrix, distCoeffs);
	}
	else
	{
		_reporter->Report(MError, "Calibration not successful");
	}
	return true;
}

bool ImageProcessor::CalibrateStep()
{
	return true;
}

bool ImageProcessor::CreateChessboards()
{
	_chesspoints.clear();
	//TODO  this should be later autodetected
	int chessW = 7, chessH = 7;
#define COEF 10
	for (int i = 0; i < chessH; i++)
	{
		for (int j = 0; j < chessW; j++)
		{
			_chesspoints.push_back(cv::Point3f(COEF*i, COEF*j, 0));
		}
	}
	// nothing more to do
	return false;
}

bool ImageProcessor::PerformCalibration(int chessWidth, int chessHeight, std::vector<cv::Point2f>& corners)
{
	int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;

	bool chessPattern = findChessboardCorners(_gr, cv::Size(chessWidth, chessHeight), corners, chessBoardFlags);

	if (chessPattern)
	{
		cornerSubPix(_gr, corners, cv::Size(11, 11),
			cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
		//emit EMessage(MInfo, "%d : FOUND", _provider->Position());

	}
	else
	{
		corners.clear();
	}

	return true;
}

void ImageProcessor::ApplyFeatureDetector()
{
	KeypointsArray arr;
	cv::Mat desc;
	_ffd->detect(_gr, arr);
	_ffd->compute(_gr, arr, desc);
	_foundCoords.push_back(arr);
	_foundDesc.push_back(desc);
}

bool ImageProcessor::PerformDetection()
{
	cv::Mat frame;
	KeypointsArray newKeypoints;
	cv::Mat newgr;

	// new gray leveled image
	std::vector<cv::Point2f> convertedKeypoints;
	std::vector<uchar> status;
	std::vector<float> err;
	cv::TermCriteria crit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 10, 0.03);
	DoAssert(newKeypoints.size() > 0);
	cv::calcOpticalFlowPyrLK(_gr, newgr, _foundCoords[_lastIndexSecondary-1], convertedKeypoints, status, err);

	// First, filter out the points with high error
	std::vector<cv::Point2f> right_points_to_find;
	std::vector<int> right_points_to_find_back_index;
	for (unsigned int i = 0; i < status.size(); i++) {
		if (status[i] && err[i] < 12.0) {
			// Keep the original index of the point in the
			// optical flow array, for future use
			right_points_to_find_back_index.push_back(i);
			// Keep the feature point itself
			right_points_to_find.push_back(convertedKeypoints[i]);
		}
		else {
			status[i] = 0; // a bad flow
		}
	}

	// for each right_point see which detected feature it belongs to
	cv::Mat right_points_to_find_flat = cv::Mat(right_points_to_find).
		reshape(1, right_points_to_find.size()); //flatten array

												 // feature from the next frame
	std::vector<cv::Point2f> right_features;

	cv::Mat right_features_flat = cv::Mat(right_features).reshape(1, right_features.size());
	// Look around each OF point in the right image
	// for any features that were detected in its area
	// and make a match.
	std::vector<cv::DMatch> matches;
	cv::BFMatcher matcher(CV_L2);

	std::vector<std::vector<cv::DMatch>> nearest_neighbors;
	matcher.radiusMatch(
		right_points_to_find_flat,
		right_features_flat,
		nearest_neighbors, 2.0f);
	// Check that the found neighbors are unique (throw away neighbors
	// that are too close together, as they may be confusing)
	std::set<int>found_in_right_points; // for duplicate prevention
	for (int i = 0; i < nearest_neighbors.size(); i++) {
		cv::DMatch match;
		if (nearest_neighbors[i].size() == 1) {
			match = nearest_neighbors[i][0]; // only one neighbor
		}
		else if (nearest_neighbors[i].size() > 1) {
			// 2 neighbors – check how close they are
			double ratio = nearest_neighbors[i][0].distance /
				nearest_neighbors[i][1].distance;
			if (ratio < 0.7) { // not too close
							   // take the closest (first) one
				match = nearest_neighbors[i][0];
			}
			else { // too close – we cannot tell which is better
				continue; // did not pass ratio test – throw away
			}
		}
		else {
			continue; // no neighbors... :(
		}
		// prevent duplicates
		if (found_in_right_points.find(match.trainIdx) == found_in_right_points.
			end()) {
			// The found neighbor was not yet used:
			// We should match it with the original indexing
			// of the left point
			match.queryIdx = right_points_to_find_back_index[match.queryIdx];
			matches.push_back(match); // add this match
			found_in_right_points.insert(match.trainIdx);
		}
	}

	QString str = QString::asprintf("Pruned %d/%d matches", matches.size(), nearest_neighbors.size());
	_reporter->Report(MInfo, str);

	_gr = newgr;
	_foundCoords.push_back( newKeypoints );
	return true;
}

void ImageProcessor::UseGrey()
{
	_gr.copyTo(_ret);
}

bool ImageProcessor::DrawFeatures()
{
	std::vector<cv::Point2f> points;
	cv::KeyPoint::convert(_foundCoords[_lastIndexSecondary],points);
	for (int i = 0; i < points.size(); i++)
	{
		cv::circle(_ret, points[i], 18, cv::Scalar(0.4, 200, 150),3);
	}
	return true;
}

cv::Mat ImageProcessor::GetResult()
{
	return _ret;
}

void ImageProcessor::ApplyCalibration(CalibrationSet & calibrationSet)
{
	if (calibrationSet.fx <= 0)
	{
		DoAssert(false);
		return;
	}

#if DEBUG_CAMERA
	std::vector<double> array;
	cv::Mat mat = calibrationSet.camera;
	if (mat.isContinuous()) {
		array.assign((double*)mat.datastart, (double*)mat.dataend);
	}
	else {
		for (int i = 0; i < mat.rows; ++i) {
			array.insert(array.end(), (float*)mat.ptr<uchar>(i), (float*)mat.ptr<uchar>(i) + mat.cols);
		}
	}
#endif
	cv::Mat distcoefs = cv::Mat::zeros(8, 1, CV_64F);
	distcoefs.at<double>(0) = calibrationSet.k1;
	distcoefs.at<double>(1) = calibrationSet.k2;
	distcoefs.at<double>(4) = calibrationSet.k3;
	// init rectification
	cv::Mat r,map1,map2;
	cv::Mat camera = cv::Mat::eye(3, 3, CV_64F);
	camera.at<double>(0, 0) = calibrationSet.fx;
	camera.at<double>(1, 1) = calibrationSet.fy;
	camera.at<double>(0, 2) = calibrationSet.px;
	camera.at<double>(1, 2) = calibrationSet.py;
	cv::initUndistortRectifyMap(
		camera, distcoefs, cv::Mat(),
		getOptimalNewCameraMatrix(camera, distcoefs, _ret.size(), 1, _ret.size(), 0), _ret.size(),
		CV_16SC2, map1, map2);
	cv::Mat cl = _ret.clone();
#if SHOW_UNDISTORTED
	imshow("First", cl);
#endif
	remap(cl, _ret, map1, map2, cv::INTER_LINEAR);
#if SHOW_UNDISTORTED
	imshow("After", cl);
	imshow("Second", _ret);
	cv::waitKey();
#endif
}

ImageProcessor::~ImageProcessor()
{
	Clear();
}

#define APP_FCE(name) _phases[index++] = name

void ImageProcessor::Create(const int& mode)
{
	int index = 0;
	Clear();
	switch (mode)
	{
		case ActionModePlay:
		{
			APP_FCE(&ImageProcessor::PrepareImage);
			APP_FCE(NULL);
			break;
		}
		case ActionModeCalibrate:
		{
			APP_FCE(&ImageProcessor::CreateChessboards);
			APP_FCE(&ImageProcessor::CalibrateStep);
			APP_FCE(&ImageProcessor::FinishCalibration);
			APP_FCE(NULL);
			break;
		}
		case ActionModeCreate:
		{
			APP_FCE(&ImageProcessor::FeaturesStep);
			APP_FCE(&ImageProcessor::MatchesStep);
			APP_FCE(&ImageProcessor::AutoCalibrate);
			APP_FCE(&ImageProcessor::FinishCreationStep);
			APP_FCE(&ImageProcessor::GlobalBundleAdjustment);
			APP_FCE(NULL);
			break;
		}
		case ActionModeCreateManual:
		{
			APP_FCE(&ImageProcessor::ManualFeaturesStep);
			APP_FCE(&ImageProcessor::InputWait);
			APP_FCE(&ImageProcessor::ManualMatchesStep);
			APP_FCE(&ImageProcessor::InputWait);
			APP_FCE(&ImageProcessor::FinishCreationStep);
			APP_FCE(&ImageProcessor::GlobalBundleAdjustment);
			APP_FCE(NULL);
			break;
		}
		default:
		{
			_reporter->Report(MError, "Nothing create :-o");
		}
	}
}

void ImageProcessor::Clear()
{
	_phase = 0;
	_modified = 0;
}

bool ImageProcessor::CleanModifiedFlag()
{
	if (!_modified)
		return false;
	_modified = false;
	return true;
}

void ImageProcessor::ProcessContext(const PointsContext& context)
{
	DoAssert(_signalAccepted);
	(this->*_signalAccepted)(context);
	// accepted
	_signalAccepted = NULL;
}


void ImageProcessor::ActionDone()
{
	_signalAccepted = NULL;
	_lastIndex = 0;
	_lastIndexSecondary = 1;
	_phase++;
}

bool ImageProcessor::FinishCalibration(PointsArray & chesspoints, cv::Mat& cameraMatrix, cv::Mat& distCoeffs)
{
	int flags = cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5;

	cv::Mat rvecs, tvecs;

	PointsArray2 chesses;
	chesses.resize(_foundCoords.size(), chesspoints);
	bool calibrated = _foundCoords.size() && cv::calibrateCamera(chesses, _foundCoords, _ret.size(), cameraMatrix, distCoeffs, rvecs, tvecs, flags);
	return calibrated;
}

#define DISCOEF 0.8
#define INLINER_THRESHOLD 0.01

// created matches between all images so far
bool ImageProcessor::MatchesStep()
{
	KeypointsArray & currentKeypoints = _foundCoords.back();
	KeypointsArray matched1, matched2;
	int& currentImage = _lastIndexSecondary;
	int& iImage = _lastIndex;
	QString message = QString::asprintf("Matching %d %d",_lastIndex, _lastIndexSecondary);
	_reporter->Report(MInfo, message.toStdString().c_str());
	//for ( int iImage =0; iImage < currentImage; iImage++)
	{
		cv::BFMatcher matcher(cv::NORM_HAMMING);
		std::vector< std::vector<cv::DMatch> > nn_matches;

		// we want exact matches, but have 2 of them for comparison
		matcher.knnMatch(_foundDesc[iImage],_foundDesc[currentImage], nn_matches, 2);
		if ( nn_matches.empty() )
			return false;
		int inliers = 0;
		// bool for double matches
		bool * testDouble[2];
		testDouble[0] = new bool[currentKeypoints.size()];
		testDouble[1] = new bool[_foundCoords[iImage].size()];

		// clear up the indices
		memset(testDouble[0], 0, currentKeypoints.size() * sizeof(testDouble[0][0]));
		memset(testDouble[1], 0, _foundCoords[iImage].size() * sizeof(testDouble[0][0]));
		std::vector<cv::Point2f> test1, test2;
		std::vector<int> itest1, itest2;

		for (int iMatch = 0; iMatch < nn_matches.size(); iMatch++)
		{
			cv::DMatch &matchProbable = nn_matches[iMatch][0];
			cv::DMatch &matchSecondProbable = nn_matches[iMatch][1];
			// reasonable ration to consider this as valid match, that' why we need two matched. To detect of the one is distinct enough
			if (matchProbable.distance > DISCOEF * matchSecondProbable.distance)
				continue;

			// already matched
			if (testDouble[0][matchProbable.trainIdx] || testDouble[1][matchProbable.queryIdx])
				continue;

			testDouble[0][matchProbable.trainIdx] = true;
			testDouble[1][matchProbable.queryIdx] = true;

			test1.push_back(currentKeypoints[matchProbable.trainIdx].pt);
			itest1.push_back(matchProbable.trainIdx);
			
			test2.push_back(_foundCoords[iImage][matchProbable.queryIdx].pt);
			itest2.push_back(matchProbable.queryIdx);
		}

		std::vector<uchar> status;
		cv::findFundamentalMat(test1, test2, status);
		//	// check if after homography the distance is close enough
		//	cv::Mat test = cv::Mat::ones(3, 1, CV_64F);
		//	test.at<double>(0) = currentKeypoints[match1.queryIdx].pt.x;
		//	test.at<double>(1) = currentKeypoints[match1.queryIdx].pt.y;
		//	test = hom * test;
		//	test /= test.at<double>(2);// normalize

		//	cv::Mat test2 = cv::Mat::ones(3, 1, CV_64F);
		//	test2.at<double>(0) = currentKeypoints[match1.trainIdx].pt.x;
		//	test2.at<double>(1) = currentKeypoints[match1.trainIdx].pt.y;

		//	test2 = test2 - test;
		//	if (sqrt(test2.dot(test2)) < INLINER_THRESHOLD)
		for (int a = 0; a < status.size(); a++)
		{
			if (status[a] == false)
				continue;
			inliers++;
			AddPair(iImage, itest2[a], _foundCoords.size() - 1, itest1[a]);			
		}
		RelMap mp;
		mp.rel = nn_matches.size() ? (double)inliers / nn_matches.size() : 0;
		mp.image1 = currentImage;
		mp.image2 = iImage;
		_reliability.push_back(mp);
	}
	iImage++;
	if (iImage == currentImage)
	{
		iImage = 0;
		currentImage++;
		if (currentImage >= _imagesInfo.size())
		{
			return false;
		}
	}
	return true;
}

#include "Misc.h"

cv::Point3f Triangulate(cv::Mat p1, cv::Mat p2, cv::Point2f id1, cv::Point2f id2)
{
	// minor bundle adjustment
	// find such P that it is observed by camera p1 at point id1 and camera p2 at point id2

	// using linear method ( see http://users.cecs.anu.edu.au/~hartley/Papers/triangulation/triangulation.pdf for more methods )

	// we minimize ( |Ax - b| = 0, as usual )
	cv::Mat A = cv::Mat::zeros(4, 3, CV_64F);
	cv::Mat b = cv::Mat::zeros(4, 1, CV_64F);
	
	A.row(0) = p1.col(2)*id1.x - p1.col(0);
	A.row(1) = p1.col(2)*id1.y - p1.col(1);
	A.row(2) = p2.col(2)*id2.x - p2.col(0);
	A.row(3) = p2.col(2)*id2.y - p2.col(1);

	cv::Mat x;
	cv::solve(A, b, x);
	cv::Point3f ret(x);
	
	return ret;
}

void ImageProcessor::PrepareCalibration()
{
	_reporter->Report( MInfo, "Preparing calibration");
	// TODO calibrate stuff

	// we have matches from the same source, we can calibrate the one camera
	for (int i = 0; i < _cameras.size(); i++)
	{
		// we made it for all camera;
		DoAssert(_cameras[i]->calibrated);
	}
	_reporter->Report( MInfo, "Calibration according to images done");
}

void ImageProcessor::FindSecondProjection(cv::Mat essential, int image )
{
	cv::Mat u, v,dummy;

	// we don't care about W, we will replace it by two matrices, that together make diagonal 2 dimensional matrix
	cv::SVD::compute(essential, dummy, u, v, cv::SVD::FULL_UV);
	//E = SR, where R is rotational matrix ans S is antisymmentric matrix
	cv::Mat w = cv::Mat::zeros(3, 3, CV_64F), z = cv::Mat::zeros(3, 3, CV_64F);

	/*z.at<double>(0, 1) = -1;
	z.at<double>(1, 0) = 1;*/

	// matrix later used for rotation matrix
	w.at<double>(0, 1) = 1;
	w.at<double>(1, 0) = -1;
	w.at<double>(2, 2) = 1;
	cv::Mat vtransposed,wtransposed;
	cv::transpose(v, vtransposed);
	cv::transpose(w, wtransposed);

	cv::Mat p[4];
	for (int i = 0; i < 3; i++)
	{
		p[i] = cv::Mat::zeros(3, 3, CV_64F);
	}
	cv::Rect rect(0, 0, 3, 3);
	cv::Rect rectT(3, 0, 1, 3);
	p[0](rect) = u * w * vtransposed;;
	p[1](rect) = u * w * vtransposed;
	p[2](rect) = u * wtransposed * vtransposed;
	p[3](rect) = u * wtransposed * vtransposed;

	cv::Mat position[4] = { cv::Mat(1,3,CV_64F), cv::Mat(1,3,CV_64F), 
		cv::Mat(1,3,CV_64F), cv::Mat(1,3,CV_64F) };
	cv::Mat neg =-u;
	u.col(2).copyTo(position[0]);
	neg.col(2).copyTo(position[1]);
	u.col(2).copyTo(position[2]);
	neg.col(2).copyTo(position[3]);

	int score[4];
	memset(score, 0, sizeof(score));
	double a0[] = {
		p[0].at<double>(0,0), p[0].at<double>(0,1), p[0].at<double>(0,2),p[0].at<double>(0,3),
		p[0].at<double>(1,0), p[0].at<double>(1,1), p[0].at<double>(1,2),p[0].at<double>(1,3),
		p[0].at<double>(2,0), p[0].at<double>(2,1), p[0].at<double>(2,2),p[0].at<double>(2,3)
	};

	double a1[] = {
		p[1].at<double>(0,0), p[1].at<double>(0,1), p[1].at<double>(0,2),p[1].at<double>(0,3),
		p[1].at<double>(1,0), p[1].at<double>(1,1), p[1].at<double>(1,2),p[1].at<double>(1,3),
		p[1].at<double>(2,0), p[1].at<double>(2,1), p[1].at<double>(2,2),p[1].at<double>(2,3),
	};

	// first one
	cv::Mat testPoint = cv::Mat::zeros(  4,1,CV_64F);
	//last one is always one
	testPoint.at<double>(3,0) = 1;

	for (int i = 0; i < _foundCoords[image].size(); i++)
	{
		testPoint.at<double>(0, 0) = _foundCoords[image][i].pt.x;
		testPoint.at<double>( 1,0) = _foundCoords[image][i].pt.y;

		for (int iCamera = 0; iCamera < 4; iCamera++)
		{
			// we need to check which one of these is these solutions is the correct one
			// the final projection matrix should give most of the points before camera
			cv::Mat ret = p[iCamera] * testPoint;
			double tp[] = { testPoint.at<double>(0,0), testPoint.at<double>(1,0), testPoint.at<double>(2,0), testPoint.at<double>(3,0) };
			double tst = ret.at<double>(2, 0);
			score[iCamera] += Sign(tst);
		}
	}
	int ret = 0;
	for (int i = 1; i < 4; i++)
	{
		if (score[i] > score[ret])
			ret = i;
	}
	SplitMatrix(p[ret], _imagesInfo[image], _imagesInfo[image]._camera, position[ret] );
}

void ImageProcessor::SplitMatrix(cv::Mat projection, ImageInfo&imageInfo, CameraParams *camera, cv::Mat position)
{
	// RQ decomposition
	imageInfo._estimated = true;
	cv::Mat K, R;
	cv::RQDecomp3x3(projection, K, R);
	if (camera->calibrated)
	{
		// check if we can update the calibration
		//TODO
	}
	camera->insistric = K;
	imageInfo._extrinsic = R;
	imageInfo._position = position;
	camera->calibrated = true;
}

bool ImageProcessor::GlobalBundleAdjustment()
{
	return false;
}

void ImageProcessor::PrepareDouble(const int& first, const int & second)
{
	cv::Mat f, s;
	_provider->Get(first,f);
	_provider->Get(second,s);
	// merge to one
	cv::Size s1 = f.size();
	cv::Size s2 = s.size();
	if (s1.height < s2.height)
		s1.height = s2.height;
	s1.width += s2.width;
	_ret = cv::Mat(s1, CV_8UC3);
	s1 = f.size();
	cv::Mat left(_ret, cv::Rect(0, 0, s1.width, s1.height));
	cv::Mat right(_ret, cv::Rect(s1.width,0, s2.width, s2.height));
	s.copyTo(left);
	f.copyTo(right);
	_modified = true;
}

struct SerializeFeaturesCallback
{
	SerializeKeyPoints serializeKeypoints;
};

bool ImageProcessor::ManualFeaturesStep()
{
	if (!_provider->Next())
		return false;
	_lastIndexSecondary = _imagesInfo.size();
	PrepareImage();
	// fill context with everything that you have
	QString nm = _provider->Name() + ".keys";
	FILE* file = fopen(nm.toStdString().c_str(), "rb");
	if (file)
	{
		std::vector<cv::KeyPoint> points;
		LoadArrayKeypoint(file, points);
		cv::KeyPoint::convert(points, _context.p1);
		// false because this can be changed
		_context.provided = false;
		fclose(file);
	}
	_signalAccepted = &ImageProcessor::InputFeatures;
	return false;
}

bool ImageProcessor::InputWait()
{
	if (_signalAccepted)
		return true; 
	// move the index
	_lastIndex++;
	if (_lastIndex == _imagesInfo.size())
	{
		_lastIndexSecondary++;
		if (_lastIndexSecondary == _imagesInfo.size() - 1)
		{
			_lastIndexSecondary = 1;
			return true;
		}
		_lastIndex = 0;
	}
	_phase--;
	return true;
}
void ImageProcessor::InputMatches(const PointsContext & context)
{
	// in points context we have all the correspondences
	for (int i = 0; i < context.p1.size(); i++)
	{
		int sec = context.p1[i].x;
		int sec2 = context.p1[i].y;
		AddPair(_lastIndex, sec, _lastIndexSecondary, sec2);
		RelMap r;
		r.image1 = _lastIndex;
		r.image2 = _lastIndexSecondary;
		// default when we did not check fundamental matrix
		r.rel = 1;
		_reliability.push_back(r);
	}
}

bool ImageProcessor::ManualMatchesStep()
{
	PrepareDouble(_lastIndex, _lastIndexSecondary);
	_signalAccepted = &ImageProcessor::InputMatches;
	return false;
}

void ImageProcessor::AddPair(const int& iImage, const int& iPoint, const int&jImage, const int & jPoint)
{
	// we can use this, create a match
	MatchPair * p1 = new MatchPair;
	p1->imageId = iImage;
	p1->pointdId = iPoint;
	MatchPair * p2 = new MatchPair;
	p2->imageId = jImage;
	p2->pointdId = jPoint;
	p1->paired = p2;
	p2->paired = p1;
	_matches.push_back(p1);
	_matches.push_back(p2);
}

// create whole 3d model
bool ImageProcessor::FinishCreationStep()
{
	// we have now tracks
	ImageProcessor::_reporter->Report(MInfo, "Generating tracks..");

	// pick up the initial camera. Two of the images must contains the most matches
	int &image1 = _lastIndexSecondary, &image2=_lastIndex;
	int camerasUsed = FindNextBestPair(image1, image2);

	// this is our first stable camera. Lets reconstruct some 3Dpoint from it
	int nframes = _foundCoords.size();

	if (camerasUsed == 0)
		return false;

	// let's have all points in images that correspond to this camera
	CoordsArray points1, points2;
	std::vector<int> matches_ids;
	for (int i = 0; i < _matches.size(); i++)
	{
		if (_matches[i]->imageId == image1 &&
			_matches[i]->paired->imageId == image2)
		{
			points1.push_back(_foundCoords[image1][_matches[i]->pointdId].pt);
			points2.push_back(_foundCoords[image2][_matches[i]->pointdId].pt);
			matches_ids.push_back(i);
		}
	}
	// estimate position
	cv::Mat F = cv::findFundamentalMat(cv::Mat(points1), cv::Mat(points2), cv::FM_8POINT, 0, 0);
	//single value decomposition for reducing rank to 2;
	//we know that fundamental matrix is rank and the diagonal number are in decreasing order
	cv::Mat u, vt, w;
	cv::SVD::compute(F, w, u, vt, cv::SVD::FULL_UV);
	cv::Mat rec = cv::Mat::zeros(3, 3, CV_64F);
	rec.at<double>(0, 0) = w.at<double>(0, 0);
	rec.at<double>(1, 1) = w.at<double>(1, 0);
	//rec.at<double>(2, 2) = w.at<double>(2, 0);
	w = rec;

	//compute once again F, with correct dimension
	F = u*w*vt;

	// now we have F that is close enough

	// now compute essential matrix between these two images, so we would take internal calibration into account
	// TODO is this the correct essential matrix?
	cv::Mat E = _imagesInfo[image1]._camera->insistric * F * _imagesInfo[image1]._camera->insistric;

	// calculate projection matrix from essential matrix

	// first if the trivial one
	
	//second one we need to compute
	FindSecondProjection(E, image2);

	//cv::Point3f ep1, ep2;

	//// find epipolar in the second image
	//cv::Point3f epipolar = GetEpipoles(F, ep1, ep2);
	//cv::Mat epMat = CreateCrossMatrix(epipolar);
	//// Projective matrices
	//// first matrix is identity
	//cv::Mat p2 = cv::Mat::eye(3, 4, CV_64F);
	////second matrix is based on the epipole
	//cv::Mat mult = epMat.mul(F);
	//cv::Rect rect(0, 0, 3, 3);
	//p2(rect)= mult;

	// make it correct according to rest of the cameras 
	// make it complete with all the other projections that we already have
	// here we assume that all P are normalized
	cv::Mat p1 = _imagesInfo[image1]._extrinsic;
	cv::Mat p2 = _imagesInfo[image1]._extrinsic;

	// TODO check
	cv::Mat position = _imagesInfo[image1]._position + _imagesInfo[image2]._position;
	// Do triangulation ( two- view now. We will check all of them later )
	// create Matrix A and B, 
	int n = 2;
	cv::Mat A = cv::Mat::zeros(2 * n, 3, CV_64F);
	cv::Mat b = cv::Mat::zeros(2 * n, 1, CV_64F);
	cv::Mat X = cv::Mat::zeros(3, 1, CV_64F);

	cv::Rect rct1(0, 0, 3, 3);
	cv::Rect transl(0, 0, 1, 3);
	cv::Mat projMatrices[] = { p1, p2 };

	// update p1/p2 base on the minimum distance to the image point
	// create for allVertices
	// first camera is always lower
	for (int i = 0; i < matches_ids.size(); i++)
	{
		int image_pid = _matches[matches_ids[i]]->imageId;
		int image_pid2 = _matches[matches_ids[i]]->paired->imageId;
		DoAssert(image_pid < _imagesInfo.size());
		DoAssert(image_pid2 < _imagesInfo.size());
		cv::Point2f pd[] = { _foundCoords[image1][image_pid].pt,
			_foundCoords[image1][image_pid2].pt };
		// set A
		for (int iProj = 0; iProj < n; iProj++)
		{
			cv::Mat & P = projMatrices[iProj];
			A.at<double>(iProj * 2, 0) = P.at<double>(0, 0) - P.at<double>(2, 0)*pd[iProj].x;
			A.at<double>(iProj * 2, 1) = P.at<double>(0, 1) - P.at<double>(2, 1)*pd[iProj].x;
			A.at<double>(iProj * 2, 2) = P.at<double>(0, 2) - P.at<double>(2, 2)*pd[iProj].x;
			A.at<double>(iProj * 2 + 1, 0) = P.at<double>(1, 0) - P.at<double>(2, 0)*pd[iProj].y;
			A.at<double>(iProj * 2 + 1, 1) = P.at<double>(1, 1) - P.at<double>(2, 1)*pd[iProj].y;
			A.at<double>(iProj * 2 + 1, 2) = P.at<double>(1, 2) - P.at<double>(2, 2)*pd[iProj].y;

			// set B
			b.at<double>(iProj * 2) = P.at<double>(2, 3)*pd[iProj].x - position.at<double>(0, 3);
			b.at<double>(iProj * 2 + 1) = P.at<double>(2, 3)*pd[iProj].y - position.at<double>(1, 3);
		}
		cv::solve(A, b, X, cv::DECOMP_SVD);
		cv::Point3f nPoint = X;// Triangulate(p1, p2, pid1, pid2);
		int index = _matches[matches_ids[i]]->pointdId;

		DoAssert(_detectorOutput);
		QString o = QString::asprintf("Generated point %d (estimated as %.3f %.3f %.3f) ", index, nPoint.x, nPoint.y, nPoint.z);
		_reporter->Report(MInfo, o);
		_detectorOutput->PointCallback(nPoint, index);
		/*if (_cameraAdjusted)
			_cameraAdjusted(rotation, position, point);*/
		/*CameraParams * param = _imagesInfo[image1]._camera;
		SplitMatrix(p1, _imagesInfo[image1], param,_im);
		param = _imagesInfo[image1]._camera;
		SplitMatrix(p2, _imagesInfo[image2], param);*/
	}
	camerasUsed += FindNextBestPair(image1, image2);

	return true;
}
