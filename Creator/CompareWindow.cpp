#include "CompareWindow.h"
#include <QPainter>

//TODO size of the pixes by settings
#define DESIRED_POPUP_SIZE	50

void CompareWindow::SetImage(cv::Mat image)
{
	_image = image;
	update();
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
	float ratio = s.width() / (float)_image.size().width;
	cv::Size newSize(_image.size());
	{
		float ratio2 = s.height() / (float)_image.size().height;
		if (ratio2 < ratio)
		{
			ratio = ratio2;
		}
		newSize.width *= ratio;
		newSize.height *= ratio;
	}
	cv::resize(_image, ret,newSize);

	if (_paintMode == PaintArea)
	{
		//new size
		cv::Point2f originalPoint(_coords.x() / ratio,_coords.y()/ratio);
		cv::Size ss(DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE);
		// convert the point to the point of original image
		cv::Rect rect(originalPoint, ss);
		cv::Mat rected = _image(rect);
		// merge this with the rect
		cv::Rect ret_rect(_coords.x(), _coords.y(), DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE);
		rected.copyTo(ret(ret_rect));
	}

	QImage::Format  format = QImage::Format_RGB888;

	cv::MatStep step = ret.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(ret.data, ret.cols, ret.rows, ret.step, format);

	painter.drawImage(0,0,image);
}
