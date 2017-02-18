#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"
#include "ReportFunctions.h"
#include <qevent.h>

void VideoRenderer::paintEvent(QPaintEvent *)
{
	if (_img.empty())
	{
		// do not paint
		return;
	}

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

	for (int i = 0; i < _pointsContext.p1.size(); i++)
	{
		cv::circle(matched, _pointsContext.p1[i]*_scale,5,cv::Scalar(0,0,255),2);
	}
	for (int i = 1; i < _pointsContext.indexes.size(); i+=2)
	{
		int first = _pointsContext.indexes[i-1];
		int second = _pointsContext.indexes[i];
		cv::line(matched, _pointsContext.p1[first]*_scale, _pointsContext.p1[second]*_scale, cv::Scalar(0, 255, 0), 2);
	}

	if (_pointsContext.indexes.size() & 1)
	{
		int i = _pointsContext.indexes.back();
		cv::circle(matched, _pointsContext.p1[i] * _scale, 5, cv::Scalar(1, 1, 1), -2);
	}

	DoAssert(_offsetx >= 0);
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
				_pointsContext.Clear();
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

void VideoRenderer::setImage(cv::Mat img,PointsContext context)
{
	_img = img.clone();
	_pointsContext = context;
	_offsetx = _offsety = 0;
	QSize s = size();
	cv::Size ss = img.size();
	float s1 = (float) s.width() / ss.width;
	float s2 = (float) s.height() / ss.height;
	_scale = qMin(s1, s2);
	update();
}

#include "Misc.h"

// zoom
void VideoRenderer::wheelEvent(QWheelEvent * event)
{
	if (_img.empty())
		return;

	// new size
	double scale = event->delta() / 140.f;
	if (scale < 0)
		scale = -1.0 / scale;
	_scale *= scale;
	if (_scale > 1)
	{
		emit reportSignal(MWarning, "Zoom detoriation!");
	}
	CheckBoundary<float>(_scale, 0.1, 1);
	_offsetx = scale * _offsetx;
	DoAssert(_offsetx >= 0);
	_offsety = scale * _offsety;
	DoAssert(_offsety >= 0);
	int nh = _img.size().height * _scale - size().height();
	int nw = _img.size().width * _scale - size().width();
	CheckBoundary<int>(_offsetx, 0, nw);
	CheckBoundary<int>(_offsety, 0, nh);
	//QPoint p = event->pos();
	//p = QWidget::mapFromGlobal(p);
	update();
}

void VideoRenderer::focusOutEvent(QFocusEvent* event)
{
	_canAccept = false;
}

void VideoRenderer::mousePressEvent(QMouseEvent *ev)
{
	_startingMousePos = ev->pos();
	_mousePressed = true;
}

void VideoRenderer::mouseMoveEvent(QMouseEvent *ev)
{
	if (!_mousePressed || _img.empty())
		return;

	//if left is pressed, move image
	if (ev->buttons() & Qt::RightButton)
	{
		_offsetx += _startingMousePos.x() - ev->pos().x();
		_offsety += _startingMousePos.y() - ev->pos().y();
		int nh = _img.size().height * _scale - size().height();
		int nw = _img.size().width * _scale - size().width();
		CheckBoundary<int>(_offsetx, 0, qMax(0, nw));
		CheckBoundary<int>(_offsety, 0, qMax(0, nh));
		_startingMousePos = ev->pos();
		update();
	}
}

void VideoRenderer::mouseReleaseEvent(QMouseEvent *ev)
{
	if (_canAccept == false)
	{
		// gained focud
		_canAccept = true;
		return;
	}
	_mousePressed = false;
	if (ev->buttons()& Qt::RightButton)
		return;// just moving around
	// generate click and pretend that this is the feature
	//only in step one
	if (_pointsContext.provided == false)
	{
		// terribly non-acurate:-/ but hey, user input...
		QPoint p = ev->pos();
		//p = QWidget::mapFromGlobal(p);
		cv::Point2f r(_offsetx + p.x()/_scale, _offsety + p.y()/_scale);
		if (r.x > _img.size().width || r.y > _img.size().height)
		{
			return;
		}
		int index = ArrayContains(r, _pointsContext.p1, 1.0/_scale);
		if (index >= 0)
			_pointsContext.p1.erase(_pointsContext.p1.begin() + index);
		else
			_pointsContext.p1.push_back(r);
		update();
	}
	else
	{
		// move or delete the point
		//find the nearest point to change
		int candidate = -1;
		// tolerance 5 pixels
		// move to the 
		//float dist = 25.0/(_scale*_scale);
		cv::Point2f c;
		c.x = ev->pos().x() + _offsetx;
		c.y = ev->pos().y() + _offsety;

		c /= _scale;
		// make
		candidate = ArrayContains(c, _pointsContext.p1, 1.0 / _scale);
		if (candidate >= 0)
		{
			if ((!_pointsContext.indexes.empty()) && 
				candidate == _pointsContext.indexes.back())
			{
				_pointsContext.indexes.pop_back();
			}
			else
			{
				// mark this candidate red and wait for the second input
				_pointsContext.indexes.push_back(candidate);
			}			
			update();
		}
	}

}

void VideoRenderer::keyPressEvent(QKeyEvent *ev)
{
	SwitchKeys(ev);
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	_canAccept = false;
	_scale = 1;
	_offsetx = 0;
	_offsety = 0;
	setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::ClickFocus);
	_pointsContext.Clear();
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
