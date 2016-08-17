#pragma once

#include <QString>
#include<QObject>
#include "opencv2/highgui/highgui.hpp"
#include <QBasicTimer>
#include <opencv2\core\core.hpp>

#include "opencv2\features2d.hpp"
#include "imageprovider.h"
#include <QThread>
#include <stack>
#include <QImage>
#include "worker.hpp"

class FrameProcessor : public QObject
{
	Q_OBJECT
		;
	std::stack<cv::Mat> _images;
	Worker _worker;
protected:
	// main thread doing openCV stuff
	QThread _thread;

	// provider 
	IImageProvider * _provider;

	//QColorcolor = np.random.randint(0, 255, (100, 3))
	// timer for sending images to show
	QBasicTimer _timer;
	
public slots:
	void ThreadStopped();
	void ImageReported(cv::Mat image);

signals :
	void imageReadySignal(cv::Mat image);
	void reportSignal(MessageLevel level, const QString& string);
	// nothing cool, just calibrate according to video/image. Signal when result is ready
	void calibrationResult(int errorcode, cv::Mat calibration);
	void finishedSignal();

public slots:
	void Report(MessageLevel level, const QString & message);

private:
	// scale to get this to the QT label size
	int _seconds;

public:
	//load
	Worker * GetWorker() { return &_worker; }
	void StartDetecting();
	void StartCalibration();
	bool Load(const QString & str);

	void timerEvent(QTimerEvent * ev);

	void ShowCurrent();
	FrameProcessor();
	~FrameProcessor();
	void Pause();
	void Stop();
	void Request(int frames);

	void SwitchMode(int flag);
};