#include "CompareWindow.h"
#include <QPainter>
#include "debug.h"

//TODO size of the pixes by settings
const int DESIRED_POPUP_SIZE = 25;

void CompareWindow::SetImage(cv::Mat image)
{
	cv::copyMakeBorder(image, _image, DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	newSizeWithBorders = _image.size();
	cv::Size imageSize = _image.size();

	QSize viewportSize = size();
	// viewport should be shown without border
	float ratio = (viewportSize.width() + DESIRED_POPUP_SIZE * 2) / (float)(imageSize.width);
	{
		float ratio2 = (viewportSize.height() + DESIRED_POPUP_SIZE * 2) / (float)(imageSize.height);
		if (ratio2 < ratio)
		{
			ratio = ratio2;
		}
		newSizeWithBorders.width *= ratio;
		newSizeWithBorders.height *= ratio;
	}

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

	QPointF candidate = event->localPos() + QPointF(DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE);
	if ((candidate.x() + DESIRED_POPUP_SIZE*2 >= newSizeWithBorders.width)
		|| (candidate.y() + DESIRED_POPUP_SIZE*2 >= newSizeWithBorders.height)
		|| (candidate.x() < DESIRED_POPUP_SIZE)
		|| (candidate.y() < DESIRED_POPUP_SIZE)
		)
		return;

	_coords = candidate;
	update();
}

void CompareWindow::paintEvent(QPaintEvent * event)
{
	if (_image.empty() )
	{
		// pr paint default image
		return;
	}

	// paint the image
	QPainter painter(this);
	QSize viewportSize = size();

	cv::Size imageSize = _image.size();
	cv::Mat showImage;

	cv::resize(_image, showImage, newSizeWithBorders);

	//TODO move to RGB
	if (_paintMode == PaintArea)
	{
		float ratio = showImage.size().width / (float)(imageSize.width);

		//new size
		cv::Point2f originalPoint(_coords.x() / ratio - DESIRED_POPUP_SIZE, _coords.y()/ratio - DESIRED_POPUP_SIZE);
		
		cv::Size ss(DESIRED_POPUP_SIZE*2, DESIRED_POPUP_SIZE*2);
		// convert the point to the point of original image
		cv::Rect rect(originalPoint, ss);
		
		cv::Mat rected = _image(rect).clone();
		// merge this with the rect
		cv::Rect ret_rect(_coords.x(), _coords.y(), DESIRED_POPUP_SIZE*2, DESIRED_POPUP_SIZE*2);
		cv::circle(rected, cv::Point2f(DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE), 3, cv::Scalar(0, 255, 0), 2);
		rected.copyTo(showImage(ret_rect));
	}

	QImage::Format  format = QImage::Format_RGB888;

	cv::Rect showRect(DESIRED_POPUP_SIZE, DESIRED_POPUP_SIZE, showImage.size().width - DESIRED_POPUP_SIZE*2, showImage.size().height - DESIRED_POPUP_SIZE*2);
	DoAssert(showRect.width == size().width() || showRect.height == size().height());
	cv::Mat showMat = showImage(showRect);
	cv::MatStep step = showMat.step;
	if (step[1] == 1)
	{
		format = QImage::Format_Grayscale8;
	}
	const QImage image(showMat.data, showMat.cols, showMat.rows, showMat.step, format);

	painter.drawImage(0,0,image);
}
