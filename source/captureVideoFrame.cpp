#include "CaptureVideoFrame.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <QTimer>
#include <QTimerEvent>
#include "debug.h"


void CaptureVideoFrame::StartDetecting()
{
	_ffd = cv::FastFeatureDetector::create();
	Fill(_frame, _keypoints, _gr, _currentPoints);

	_timer.start(_seconds, this);
}

bool CaptureVideoFrame::Load(const QString & str)
{
	_capture.reset( new cv::VideoCapture(str.toStdString()));
	if (!_capture->isOpened())
	{
		DoAssert(false);
		return false;
	}
	// params for ShiTomasi corner detection

	_frameH = _capture->get(CV_CAP_PROP_FRAME_HEIGHT);
	_frameW = _capture->get(CV_CAP_PROP_FRAME_WIDTH);
	_fps = _capture->get(CV_CAP_PROP_FPS);
	_numFrames = _capture->get(CV_CAP_PROP_FRAME_COUNT);
	//todo more precise
	_seconds = 1000 / _fps;
	_colors = cv::Scalar(0, 1, 0);

	return true;
}

#include "cvhelpers.h"

void CaptureVideoFrame::Fill(cv::Mat&frame, std::vector<cv::KeyPoint>& corners, cv::Mat& gr, std::vector<cv::Point2f>&points )
{
	_capture->read(frame);

	cv::cvtColor(frame, gr, CV_BGR2GRAY);
	_ffd->detect(gr, corners);
	KeypointsToPoints(corners, points);
	//cv::goodFeaturesToTrack(gr, corners, 100, 0.1, 10, cv::noArray(), 7);
}

#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>

static void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }

#include <set>

void CaptureVideoFrame::StartCalibration()
{
	calibrate();
}

void CaptureVideoFrame::calibrate() {

}

void CaptureVideoFrame::timerEvent(QTimerEvent * ev) {
	if (ev->timerId() != _timer.timerId()) 
		return;
	
	cv::Mat frame;

	std::vector<cv::KeyPoint> newKeypoints;

	cv::Mat newgr;
	// new gray leveled image
	std::vector<cv::Point2f> convertedKeypoints;

	Fill(frame, newKeypoints, newgr, convertedKeypoints);
	
	std::vector<uchar> status;
	std::vector<float> err;
	cv::TermCriteria crit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 10, 0.03);
	DoAssert(newKeypoints.size() > 0);
	cv::calcOpticalFlowPyrLK(_gr, newgr, _currentPoints, convertedKeypoints, status, err);
	
	// First, filter out the points with high error
	std::vector<cv::Point2f> right_points_to_find;
	std::vector<int> right_points_to_find_back_index;
	for (unsigned int i = 0; i < status.size(); i++) {
		if ( status[i] && err[i] < 12.0) {
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
	KeypointsToPoints( _keypoints, right_features);
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
			// 2 neighbors � check how close they are
			double ratio = nearest_neighbors[i][0].distance /
				nearest_neighbors[i][1].distance;
			if (ratio < 0.7) { // not too close
							   // take the closest (first) one
				match = nearest_neighbors[i][0];
			}
			else { // too close � we cannot tell which is better
				continue; // did not pass ratio test � throw away
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

	gf_report(MInfo,"Pruned %d/%d matches", matches.size(), nearest_neighbors.size());

	_gr = newgr;
	_keypoints = newKeypoints;
	
	cv::resize( frame, _frame, cv::Size(_xSize,_ySize));
	cv::cvtColor(_frame, _frame, CV_BGR2RGB);

	ShowCurrent();
}

void CaptureVideoFrame::ShowCurrent()
{
	cv::Mat ret;
	QImage::Format format;
	if (_mode&ModeGrey)
	{
		ret = _gr;
		format = QImage::Format_Grayscale8;
	}
	else
	{
		ret = _frame;
		format = QImage::Format_RGB888;
	}

	if (_mode & ModeFeatures)
	{
		//for each that ids fine, draw circle 
		for (int i = 0; i < _keypoints.size(); i++)
		{
			{
				cv::Point2f & p = _keypoints[i].pt;
				cv::circle(ret, p, 5, _colors);
			}
		}
	}

	//final image
	const QImage image(ret.data, ret.cols, ret.rows, ret.step,
		format, &matDeleter, new cv::Mat(ret));
	DoAssert(image.constBits() == ret.data);
	emit signalImageReady(image);
}

CaptureVideoFrame::~CaptureVideoFrame()
{
	_timer.stop();
	DoAssert(!_timer.isActive());
}
void CaptureVideoFrame::setFactors(const QSize& size)
{
	_xSize = size.width();
	_ySize = size.height();
}

void CaptureVideoFrame::Pause()
{
	if (!_timer.isActive())
		_timer.start(_seconds, this);
	else
	{
		_timer.stop();
		DoAssert(!_timer.isActive());
	}
}

void CaptureVideoFrame::Stop()
{
	Pause();
	_capture->set(cv::CAP_PROP_POS_FRAMES, 0);
	Fill(_frame, _keypoints, _gr,_currentPoints);
	ShowCurrent();
}

void CaptureVideoFrame::Request(int frames)
{
	if (_timer.isActive())
		return;
	double ret = _capture->get( cv::CAP_PROP_POS_FRAMES)+frames;
	if (ret < 0)
		ret = 0;
	if (ret > _numFrames)
		ret = _numFrames;
	_capture->set(cv::CAP_PROP_POS_FRAMES, ret);
	Fill(_frame, _keypoints, _gr,_currentPoints);
	ShowCurrent();
}

void CaptureVideoFrame::SwitchMode(int flag)
{
	_mode ^= flag;
}

