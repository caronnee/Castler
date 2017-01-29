#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"
#include "ReportFunctions.h"
#include <qevent.h>

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

void VideoRenderer::SwitchKeys(QKeyEvent *e)
{
	bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
	switch (e->key())
	{
	case Qt::Key_Return:
	{
		if (shiftPressed)
		{
			// phase complete signal
			emit PhaseDoneSignal();
		}
		else
		{
			// phase action complete
			emit PartialActionDoneSignal(_pointsContext);
		}
		// emit acception signal
		break;
	}
	}
}

void VideoRenderer::setImage(cv::Mat img)
{
	_img = img.clone();
	update();
}

void VideoRenderer::keyPressEvent(QKeyEvent *ev)
{
	SwitchKeys(ev);
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::ClickFocus);

	_pointsContext.p1 = cv::Point2f(0, 0);
	_pointsContext.p2 = cv::Point2f(0, 0);
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
