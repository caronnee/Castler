#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "CaptureVideoFrame.h"
#include <QThread>

class VideoRenderer : public QWidget {
	Q_OBJECT

private:
	int _mode = 0;

	CaptureVideoFrame * _capturer = NULL;
	QThread opencvVideoThread;
	QImage _img;
	void paintEvent(QPaintEvent *);

public slots:
	void ShowGreyFrame();
	void FeaturesFromFrame();
	void setImage(const QImage & img);
	void Stop();
	void Pause();
	void RequestNextFrame();
	void RequestPrevFrame();

public:
	
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	void Clean();
	~VideoRenderer();

	void Load(const QString & str);
};
