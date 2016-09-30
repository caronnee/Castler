#include "CompareWindow.h"
#include <QPainter>

void CompareWindow::SetImage(cv::Mat image)
{
	_image = image;
}

CompareWindow::CompareWindow(QWidget * parent) : QWidget(parent)
{
	_paintMode = PaintNone;
	_coords = QPointF(0, 0);
}

// this is actually dragging, until parent says otherwise
void CompareWindow::mouseMoveEvent(QMouseEvent * event)
{
	//	emit reportSignal(MInfo, "Mouse moved");
	_paintMode = PaintArea;
	_coords = event->localPos();
	update();
}

void CompareWindow::paintEvent(QPaintEvent * event)
{
	// paint the image
	QPainter painter(this);
	
	cv::Mat ret;
	QSize s = size();
	if (_image.size().width == 0)
		return;// do not draw
	cv::resize(_image, ret, cv::Size(s.width(), s.height()));

	QImage::Format  format = QImage::Format_RGB888;

	cv::MatStep step = ret.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(ret.data, ret.cols, ret.rows, ret.step, format);

	painter.drawImage(QPoint(0,0),image);
}
