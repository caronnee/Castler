#include "CaptureVideoFrame.h"
#include <opencv2/highgui/highgui.hpp>
#include <QTimer>
#include <QTimerEvent>

bool CaptureVideoFrame::Load(const QString & str)
{
	_capture.reset( new cv::VideoCapture(str.toStdString()));
	if (!_capture->isOpened())
		return false;
	_frameH = _capture->get(CV_CAP_PROP_FRAME_HEIGHT);
	_frameW = _capture->get(CV_CAP_PROP_FRAME_WIDTH);
	_fps = _capture->get(CV_CAP_PROP_FPS);
	_numFrames = _capture->get(CV_CAP_PROP_FRAME_COUNT);
	_capture->read(_frame);
	//todo more prrecise
	int seconds = 1000 / _fps;
	_timer.start(seconds,this);
	return true;
}

#include "debug.h"
#include <opencv2/opencv.hpp>

static void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }

void CaptureVideoFrame::timerEvent(QTimerEvent * ev) {
	if (ev->timerId() != _timer.timerId()) 
		return;
	
	cv::Mat frame;
	_capture->read(frame);
	cv::resize( frame, _frame, cv::Size(_xSize,_ySize));
	cv::cvtColor(_frame, _frame, CV_BGR2RGB);
	const QImage image(_frame.data, _frame.cols, _frame.rows, _frame.step,
		QImage::Format_RGB888, &matDeleter, new cv::Mat(_frame));
	DoAssert(image.constBits() == _frame.data);
	
	emit signalImageReady(image);
}

CaptureVideoFrame::~CaptureVideoFrame()
{
	_timer.stop();
}
void CaptureVideoFrame::setFactors(const QSize& size)
{
	_xSize = size.width();
	_ySize = size.height();
}

