#include "CompareWindow.h"
#include <QPainter>

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
	painter.drawImage(QPoint(0,0),_image);
}
