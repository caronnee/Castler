#pragma once

// popup size in pixels
#define DESIRED_POPUP_SIZE	20

#include <QWidget>
#include <qpoint.h>

#include <QMouseEvent>

class CompareWindow: public QWidget {
	Q_OBJECT

	enum PaintMode
	{
		PaintNone,
		PaintArea,
		NPaintModes
	};

	QImage _image;
	QPointF _coords;
	PaintMode _paintMode = PaintNone;

signals:

public slots:

public:
	// constructor
	CompareWindow(QWidget * parent);

	// possible to pis out differencies, zoom in and so on
	void mouseMoveEvent(QMouseEvent * event);

	//paints the image
	void paintEvent(QPaintEvent *event);
};