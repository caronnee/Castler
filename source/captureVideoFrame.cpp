#include "CaptureVideoFrame.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <QTimer>
#include <QTimerEvent>
#include "debug.h"

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
	_timer.start(_seconds,this);
	Fill(_frame, _oldcorners, _gr);
	return true;
}

void CaptureVideoFrame::Fill(cv::Mat&frame, std::vector<cv::Point2f>& corners, cv::Mat& gr )
{
	_capture->read(frame);

	//TODO change this later
	//corners.resize(1000);
	cv::cvtColor(frame, gr, CV_BGR2GRAY);
	cv::goodFeaturesToTrack(gr, corners, 100, 0.3, 7, cv::noArray(), 7);
}
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>

static void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }

void CaptureVideoFrame::timerEvent(QTimerEvent * ev) {
	if (ev->timerId() != _timer.timerId()) 
		return;
	
	cv::Mat frame;

	std::vector<cv::Point2f> oCorners;
	cv::Mat newgr;
	cv::Mat featuresToTrack;
	// new gray leveled image
	Fill(frame, oCorners, newgr);
	
	cv::Mat status,err;
	cv::TermCriteria crit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 10, 0.03);
	cv::calcOpticalFlowPyrLK(_gr, newgr, _oldcorners, oCorners, status, err, cv::Size(15, 15), 2, crit);
	DoAssert(oCorners.size()>0);
	int full = 0;
	{
		//for each that ids fine, draw circle 
		for (int i = 0; i < status.rows; i++)
		{
			if (status.data[i] != 0)
			{
				cv::Point2f & p = oCorners[i];
				cv::circle(frame, p, 5, _colors);
				full++;
			}
		}
	}
	gf_report(LogHandler::MInfo, "Tracking corners: Found %d, showing %d",oCorners.size(),full);

	_gr = newgr;
	_oldcorners = oCorners;
	
	cv::resize( frame, _frame, cv::Size(_xSize,_ySize));
	cv::cvtColor(_frame, _frame, CV_BGR2RGB);

	ShowCurrent();
}

void CaptureVideoFrame::ShowCurrent()
{
	if (_mode&ModeGrey)
	{
		//final image
		const QImage image(_gr.data, _gr.cols, _gr.rows, _gr.step,
			QImage::Format_Grayscale8, &matDeleter, new cv::Mat(_gr));
		DoAssert(image.constBits() == _gr.data);
		emit signalImageReady(image);
		return;
	}
	//final image
	const QImage image(_frame.data, _frame.cols, _frame.rows, _frame.step,
		QImage::Format_RGB888, &matDeleter, new cv::Mat(_frame));
	DoAssert(image.constBits() == _frame.data);
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
	Fill(_frame, _oldcorners, _gr);
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
	Fill(_frame, _oldcorners, _gr);
	ShowCurrent();
}

void CaptureVideoFrame::SwitchMode(int flag)
{
	_mode ^= flag;
}

