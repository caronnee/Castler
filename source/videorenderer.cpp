#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"
#include "ReportFunctions.h"

void VideoRenderer::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	cv::Mat matched;
	if (_img.empty())
		return;// do not paint
	QSize s = size();
	cv::resize(_img, matched, cv::Size(s.width(),s.height()));
	QImage::Format  format = QImage::Format_RGB888;
	cv::MatStep step = matched.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(matched.data, matched.cols, matched.rows, matched.step, format);
	p.drawImage(0, 0, image);
	//m_img = QImage();
}

void VideoRenderer::setImage(cv::Mat img)
{
	_img = img;
	update();
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	setAttribute(Qt::WA_OpaquePaintEvent);

	// connections
	connect(&_capturer, SIGNAL(imageReadySignal(cv::Mat)), this, SLOT(setImage(cv::Mat)));
	connect(&_capturer, SIGNAL(finishedSignal()), this, SLOT(NoMoreImages()));
	connect(&_capturer, SIGNAL(reportSignal(MessageLevel, const QString &)), this, SLOT(Report(MessageLevel, const QString &)));
	connect(_capturer.GetWorker(), SIGNAL(camParametersSignal(cv::Mat, cv::Mat)), this, SLOT(ShowParameters(cv::Mat, cv::Mat)));
	connect(this, SIGNAL(setCalibrationSignal(CalibrationSet)), _capturer.GetWorker(), SLOT(ChangeCalibration(CalibrationSet)));
//	connect(this, SIGNAL());
}

void VideoRenderer::SetParameters( CalibrationSet calibration)
{
	emit setCalibrationSignal(calibration);
}

VideoRenderer::~VideoRenderer()
{
}

void VideoRenderer::RequestPrevFrame()
{
	_capturer.Request(-2);
}

void VideoRenderer::PlayUndistorted()
{
	emit modeChanged(ModeUndistort);
}

void VideoRenderer::ShowGreyFrame()
{
	emit reportSignal(MInfo, "Showing grey frame");
	emit modeChanged(ModeGrey);
}


void VideoRenderer::FeaturesFromFrame()
{
	emit modeChanged(ModeFeatures);
}

void VideoRenderer::RequestNextFrame()
{
	_capturer.Request(0);
}

void VideoRenderer::Pause()
{
	_capturer.Pause();
}
void VideoRenderer::Stop()
{
	_capturer.Stop();
}

void VideoRenderer::Report(MessageLevel level, const QString & message)
{
	emit reportSignal(level, message);
}
void VideoRenderer::NoMoreImages()
{
	emit reportSignal(MInfo, "Done");
	emit Finished();
}

void VideoRenderer::ShowParameters(cv::Mat camera, cv::Mat dist)
{
	emit setCameraSignal(camera,0);
	emit setCameraSignal(dist,1);

}

bool VideoRenderer::Start(const QString & str, VideoAction action)
{
	switch (action)
	{
		case ActionUndistort:
		{
			return _capturer.Load(str, ModeUndistort);
		}
		case ActionCalibrate:
		{
			return _capturer.Load(str, ModeCalibrate);
		}
		case ActionDetect:
		{
			return _capturer.Load(str, ModeDetect);
		}
	}
	return true;
}

//void SetLogging(IReportFunction * handler);
//void gf_report(MessageLevel level, const char * format, ...);