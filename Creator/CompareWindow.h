#pragma once

// popup size in pixels
#include <QWidget>
#include <qpoint.h>

#include <QMouseEvent>
#include <opencv2/opencv.hpp>

class CompareWindow: public QWidget {
	Q_OBJECT

	enum PaintMode
	{
		PaintNone,
		PaintArea,
		NPaintModes
	};

	cv::Mat _image;

	QPointF _coords;

	PaintMode _paintMode = PaintNone;

signals:

	public slots :
public:
	// constructor
	CompareWindow(QWidget * parent);

	// sets which image should be used
	void SetImage(cv::Mat image);

	// possible to pis out differencies, zoom in and so on
	void mouseMoveEvent(QMouseEvent * event);

	//paints the image
	void paintEvent(QPaintEvent *event);
};