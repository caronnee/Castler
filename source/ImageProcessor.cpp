#include "ImageProcessor.h"
#include "Misc.h"

IReportFunction * ImageProcessor::_reporter = NULL;

int ImageProcessor::FindBestPair(int& firstCamera, int & secondCamera)
{
	return 0;
}

ImageProcessor::ImageProcessor()
{
	_provider = NULL;
	_colors = cv::Scalar(0, 1, 0);
	_lastImageIndex = 0;
}

bool ImageProcessor::Init(Providers * provider, bool ffd /*= false*/)
{
	_provider = provider;
	_imagesInfo.resize(_provider->Count());
	_ffd = cv::GFTTDetector::create();
	return true;
}

cv::Size ImageProcessor::GetSize()
{
	return _frameSize;
}

bool ImageProcessor::Next( )
{
	if (!_provider || _provider->Get()->NextFrame(_frame) == false)
		return false;

	_lastImageIndex++;
	_frameSize = _frame.size();
	cv::cvtColor(_frame, _gr, CV_RGB2GRAY);
	
	//cv::goodFeaturesToTrack(gr, corners, 100, 0.1, 10, cv::noArray(), 7);
	_frame.copyTo(_ret);
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
	MatArray desc;
	_ffd->detectAndCompute(_gr, cv::noArray(), arr, desc);

	_foundCoords.push_back(arr);
	_foundDesc.push_back(desc);
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
	cv::calcOpticalFlowPyrLK(_gr, newgr, _foundCoords[_lastImageIndex-1], convertedKeypoints, status, err);

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
	cv::KeyPoint::convert(_foundCoords[_lastImageIndex],points);
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

void ImageProcessor::CreateMatches()
{
	std::vector<cv::DMatch> goodMatches;
	KeypointsArray & currentKeypoints = _foundCoords.back();
	KeypointsArray matched1, matched2;
	MatArray &currentDesc = _foundDesc.back();
	for (int i = 0; i < _foundDesc.size() - 1; i++)
	{
		cv::BFMatcher matcher(cv::NORM_HAMMING);
		std::vector< std::vector<cv::DMatch> > nn_matches;
		// we want exact matches, but have 2 of them for comaprison
		matcher.knnMatch(currentDesc, _foundDesc[i], nn_matches, 2);
		int inliers = 0;
		cv::Mat hom = cv::findHomography(_foundCoords[i], currentKeypoints);
		for (int iMatch = 0; iMatch < nn_matches.size(); iMatch++)
		{
			cv::DMatch &match1 = nn_matches[iMatch][0];
			cv::DMatch &match2 = nn_matches[iMatch][1];
			// reasonable ration to consider this as valid match, that' why we need two matched. To detect of the one is distinct enough
			if (match1.distance > DISCOEF * match2.distance)
				continue;

			// check if after homography the distance is close enough
			cv::Mat test = cv::Mat::ones(3, 1, CV_64F);
			test.at<double>(0) = currentKeypoints[match1.queryIdx].pt.x;
			test.at<double>(1) = currentKeypoints[match1.queryIdx].pt.y;
			test = hom * test;
			test /= test.at<double>(2);// normalize

			cv::Mat test2 = cv::Mat::ones(3, 1, CV_64F);
			test2.at<double>(0) = currentKeypoints[match1.trainIdx].pt.x;
			test2.at<double>(1) = currentKeypoints[match1.trainIdx].pt.y;

			test2 = test2 - test;
			if (sqrt(test2.dot(test2)) < INLINER_THRESHOLD)
			{
				// TODO this wont work. Pair must be here just once
				inliers++;
				// we can use this, create a match
				MatchPair * p1 = new MatchPair;
				p1->imageId = _foundCoords.size() - 1;
				p1->pointdId = match1.queryIdx;
				MatchPair * p2 = new MatchPair;
				p2->imageId = match1.imgIdx;
				p2->pointdId = match1.trainIdx;
				p1->paired = p2;
				p2->paired = p1;
				_matches.push_back(p1);
				_matches.push_back(p2);
			}
		}
		_reliability.push_back(nn_matches.size()?0:inliers/nn_matches.size());
	}
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
		DoAssert(_cameras[i].calibrated);
	}
	_reporter->Report( MInfo, "Calibration according to images done");
}

cv::Mat ImageProcessor::FindProjection(cv::Mat essential, int image )
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
	cv::Rect rect(0, 0, 3, 3);
	cv::Rect rectT(0, 3, 3, 1);
	p[0](rect) = u * w * vtransposed;
	p[1](rect) = u * w * vtransposed;
	p[2](rect) = u * wtransposed * vtransposed;
	p[3](rect) = u * wtransposed * vtransposed;

	p[0](rectT) = u.col(3);
	p[1](rectT) = -u.col(3);
	p[2](rectT) = u.col(3);
	p[3](rectT) = -u.col(3);

	int score[4];
	memset(score, 0, sizeof(score));

	// first one
	cv::Mat testPoint = cv::Mat::zeros(4, 1, CV_64F);
	//last one is always one
	testPoint.at<double>(3, 0) = 1;

	for (int i = 0; i < _foundCoords[image].size(); i++)
	{
		testPoint.at<double>(0, 0) = _foundCoords[image][i].pt.x;
		testPoint.at<double>(1, 0) = _foundCoords[image][i].pt.y;

		for (int iCamera = 0; iCamera < 4; iCamera++)
		{
			// we need to check which one of these is these solutions is the correct one
			// the final projection matrix should give most of the points before camera
			cv::Mat ret = p[iCamera] * testPoint;
			score[iCamera] += Sign(ret.at<double>(2,0) / ret.at<double>(3, 0));
		}
	}
	int ret = 0;
	for (int i = 1; i < 4; i++)
	{
		if (score[i] > score[ret])
			ret = i;
	}
	SplitMatrix(p[ret], _imagesInfo[image]._extrinsic, _imagesInfo[image]._camera );
	return p[ret];
}
void ImageProcessor::SplitMatrix(cv::Mat projection, cv::Mat& extrinsic, CameraParams *camera)
{
	// TODO fill
	//rq decomposition?
}

void ImageProcessor::FinishCreation()
{
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

	// we have now tracks
	ImageProcessor::_reporter->Report(MInfo, "Done generating tracks");

	std::vector<int> processsedCameras;
	// pick up the initial camera. Two of the images must contains the most matches
	// and it must have good inliner ratio, from the homography calculation
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
		if (_reliability[i] > best)
		{
			best = _reliability[i];
			image2 = nframes - count;
			image1 = i - (framesCount+count)*(count+1)/2;
			DoAssert((nframes * 2 - image1)*(image1 + 1) / 2 + image2 == i);
		}
	}

	processsedCameras.push_back(image1);
	processsedCameras.push_back(image2);

	// this is our first stable camera. Lets reconstruct some 3Dpoint from it
	int camerasUsed = 2;
	
	// let's have all points in images that correspond to this camera
	while (camerasUsed < framesCount)
	{
		CoordsArray points1, points2;
		std::vector<int> matches_ids;
		for (int i = 0; i < _matches.size(); i++)
		{
			if (_matches[i]->imageId == image1)
			{
				points1.push_back(_foundCoords[image1][_matches[i]->pointdId].pt);
				matches_ids.push_back(i);
			}
			if (_matches[i]->imageId == image2)
			{
				points2.push_back(_foundCoords[image2][_matches[i]->pointdId].pt);
				matches_ids.push_back(i);
			}
		}
		// estimate position
		cv::Mat F = cv::findFundamentalMat(cv::Mat(points1), cv::Mat(points2), cv::FM_8POINT, 0, 0);
		//single value decomposition for reducing rank to 2;
		//we know that fundamental matrix is rank and the diagonal number are in decreasing order
		cv::Mat u, v, w;
		cv::SVD::compute(F, w, u, v, cv::SVD::FULL_UV);
		w.at<double>(3, 3) = 0;
		w.at<double>(2, 2) = 0;
		//compute once again F, with correct dimension
		F = u*w*v;
		// now we have F that is closed enough

		// now compute essential matrix between these two images, so we would take internal calibration into account
		cv::Mat E = _imagesInfo[image1]._camera->insistric * F * _imagesInfo[image1]._camera->insistric;

		// calculate projection matrix from essential matrix

		// forst if the trivil one
		cv::Mat p1 = cv::Mat::eye(3, 4, CV_64F);

		//second one we need to compute
		cv::Mat p2 = FindProjection(E,image2);

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

		// make it correct 
		// otherwise it is already matched or is identity
		p1 = _imagesInfo[image1]._extrinsic * p1;
		p2 = _imagesInfo[image1]._extrinsic * p2;
		
		// update p1/p2 base on the minimum distance to the image point
		// create for allVertices
		// first camera is always lower
		for (int i = 0; i < matches_ids.size(); i++)
		{
			int image_pid = _matches[matches_ids[i]]->id;
			int image_pid2 = _matches[matches_ids[i]]->paired->id;
			cv::Point2f pid1 =
				_foundCoords[image1][image_pid].pt, pid2;
			cv::Point3f nPoint = Triangulate(p1,p2, pid1, pid2);
			int index = _matches[matches_ids[i]]->pointdId;

			DoAssert(_pointCallback);
			if ( _pointCallback )
				_pointCallback(nPoint, index);
			/*if (_cameraAdjusted)
				_cameraAdjusted(rotation, position, point);*/
		}
		camerasUsed += FindBestPair(image1, image2);
	}

	// global bundle adjustments

}
