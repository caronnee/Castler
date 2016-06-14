#pragma once

#include <QString>
#include<QObject>
#include "opencv2/highgui/highgui.hpp"
#include <QBasicTimer>

class CaptureVideoFrame : public QObject
{
	Q_OBJECT

	
protected:
	QBasicTimer _timer;
	QScopedPointer<cv::VideoCapture> _capture;
	int _frameH;
	int _frameW;
	int _fps;
	int _numFrames;
	cv::Mat _frame;

public slots:
	
	
signals :
	void signalImageReady(const QImage& image);

public:
	//load
	bool Load(const QString & str);
	void timerEvent(QTimerEvent * ev);
	~CaptureVideoFrame();
	void setFactors(const QSize& size);
private:
	int _xSize;
	int _ySize;
};