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

	QPainter p(this);
	p.eraseRect(this->rect());
	cv::Mat matched;
	int h = _img.size().height * _scale;
	int w = _img.size().width * _scale;
	if (w <= 0 || h <= 0)
	{
		emit reportSignal(MInfo,"size is zero!");
		return;
	}
	QString s = QString::asprintf("Scaled with .5%f to %d %d from original %d %d", _scale, w, h, _img.size().width, _img.size().height);
	emit reportSignal(MInfo, s);
	int vh = size().height();
	int vw = size().width();
	cv::resize(_img, matched, cv::Size(w,h));
	cv::Rect r(_offsetx, _offsety, qMin(w, vw), qMin(h, vh));
	cv::Mat wnd= matched(r);
	QImage::Format format = QImage::Format_RGB888;
	cv::MatStep step = wnd.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(wnd.data, wnd.cols, wnd.rows, wnd.step, format);
	p.drawImage(0, 0, image);
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
			break;
		}
	}
}

void VideoRenderer::setImage(cv::Mat img)
{
	_img = img.clone();
	QSize s = size();
	cv::Size ss = img.size();
	float s1 = (float) s.width() / ss.width;
	float s2 = (float) s.height() / ss.height;
	_scale = qMin(s1, s2);
	update();
}
// zoom
void VideoRenderer::wheelEvent(QWheelEvent * event)
{
	if (_img.empty())
		return;
	//_lastpoint will by always regarding the original image
	// new size
	double scale = event->delta() / 140.f;
	if (scale < 0)
		scale = -1.0 / scale;
	_scale *= scale;
	_offsetx = scale * _offsetx;
	_offsety = scale * _offsety;
	//QPoint p = event->pos();
	//p = QWidget::mapFromGlobal(p);
	update();
}

void VideoRenderer::mousePressEvent(QMouseEvent *ev)
{
	_mousePressed = true;
}

void VideoRenderer::mouseReleaseEvent(QMouseEvent *ev)
{
	_mousePressed = false;
	// generate click and pretend that this is the feature
	//only in step one
	if (_pointsContext.provided == false)
	{
		_pointsContext.p1.push_back(_lastPoint);
	}
	else
	{
		// TODO implement
		__debugbreak();
	}
}

void VideoRenderer::keyPressEvent(QKeyEvent *ev)
{
	SwitchKeys(ev);
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	_scale = 1;
	_offsetx = 0;
	_offsety = 0;
	setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::ClickFocus);
	_pointsContext.p1.clear();
	_mousePressed = false;
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
