#include "ImageProcessor.h"

bool ImageProcessor::Next()
{
	return Prepare(_frame, _keypoints, _gr, _currentPoints);
}

IReportFunction * ImageProcessor::_reporter = NULL;

ImageProcessor::ImageProcessor()
{
	_provider = NULL;
	_colors = cv::Scalar(0, 1, 0);
	_ffd = nullptr;
}

bool ImageProcessor::Init(IImageProvider * provider, bool ffd /*= false*/)
{
	_provider = provider;
	if (ffd)
	{
		_ffd = cv::FastFeatureDetector::create();
	}
	return true;
}

cv::Size ImageProcessor::GetSize()
{
	return _frameSize;
}

bool ImageProcessor::Prepare( cv::Mat& frame, std::vector<cv::KeyPoint>& corners, cv::Mat& gr, std::vector<cv::Point2f>&points)
{
	if (!_provider || _provider->NextFrame(frame) == false)
		return false;

	_frameSize = frame.size();
	cv::cvtColor(frame, gr, CV_BGR2GRAY);
	cv::cvtColor(frame, frame, CV_BGR2RGB);

	if (_ffd)
	{
		_ffd->detect(gr, corners);
		cv::KeyPoint::convert(corners, points);
	}
	//cv::goodFeaturesToTrack(gr, corners, 100, 0.1, 10, cv::noArray(), 7);
	frame.copyTo(_ret);
	return true;
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
		cv::KeyPoint::convert(corners, _keypoints);
	}
	else
	{
		corners.clear();
	}

	
	return true;
}

bool ImageProcessor::PerformDetection()
{
	cv::Mat frame;

	std::vector<cv::KeyPoint> newKeypoints;

	cv::Mat newgr;
	// new gray leveled image
	std::vector<cv::Point2f> convertedKeypoints;

	std::vector<uchar> status;
	std::vector<float> err;
	cv::TermCriteria crit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 10, 0.03);
	DoAssert(newKeypoints.size() > 0);
	cv::calcOpticalFlowPyrLK(_gr, newgr, _currentPoints, convertedKeypoints, status, err);

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
	std::vector<cv::Point2f> right_features; // detected features
	cv::KeyPoint::convert(_keypoints, right_features);
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
	_keypoints = newKeypoints;
	return true;
}

void ImageProcessor::UseGrey()
{
	_gr.copyTo(_ret);
}

bool ImageProcessor::DrawFeatures()
{
	return true;
}

void ImageProcessor::ToGrey()
{
	_ret = _gr;
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
