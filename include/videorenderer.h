#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "CaptureVideoFrame.h"
#include <QThread>

class VideoRenderer : public QWidget {
	Q_OBJECT

private:
	CaptureVideoFrame * _capturer = NULL;
	QThread opencvVideoThread;
	QImage _img;
	void paintEvent(QPaintEvent *);

public slots:
	void setImage(const QImage & img);

public:
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	void Clean();
	~VideoRenderer();

	void Load(const QString & str);
};
