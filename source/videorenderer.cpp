#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"
#include "ReportFunctions.h"

void VideoRenderer::paintEvent(QPaintEvent *)
{
	if (_img.empty())
		return;// do not paint

	cv::Mat matched;

	QSize s = size();
	cv::resize(_img, matched, cv::Size(s.width(),s.height()));
	QImage::Format  format = QImage::Format_RGB888;
	cv::MatStep step = matched.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(matched.data, matched.cols, matched.rows, matched.step, format);
	QPainter p(this);
	p.drawImage(0, 0, image);
	//m_img = QImage();
}

void VideoRenderer::setImage(cv::Mat img)
{
	_img = img.clone();
	update();
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	setAttribute(Qt::WA_OpaquePaintEvent);
}

void VideoRenderer::SetParameters( CalibrationSet calibration)
{
	emit setCalibrationSignal(calibration);
}

VideoRenderer::~VideoRenderer()
{
}

void VideoRenderer::Report(MessageLevel level, const QString & message)
{
	emit reportSignal(level, message);
}

void VideoRenderer::ShowParameters(cv::Mat camera, cv::Mat dist)
{
	emit setCameraSignal(camera,0);
	emit setCameraSignal(dist,1);

}

/*bool VideoRenderer::Start(const QString & str, VideoAction action)
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
*/
//void SetLogging(IReportFunction * handler);
//void gf_report(MessageLevel level, const char * format, ...);