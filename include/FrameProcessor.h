#pragma once

#include <QString>
#include <QObject>
#include "opencv2/highgui/highgui.hpp"
#include <QBasicTimer>
#include <opencv2\core\core.hpp>

#include "opencv2\features2d.hpp"
#include "imageprovider.h"
#include <QThread>
#include <stack>
#include <QImage>
#include "ExtractionWorker.hpp"


class FrameProcessor : public QObject
{
	Q_OBJECT
		;

private:
	// scale to get this to the QT label size
	int _seconds = 0;

	std::stack<cv::Mat> _images;

	ExtractionWorker _worker;

protected:
	// main thread doing openCV stuff
	QThread _thread;
	//QColorcolor = np.random.randint(0, 255, (100, 3))
	// timer for sending images to show
	QBasicTimer _timer;
	
public slots:
	void ThreadStopped();
	void ImageReported(cv::Mat image, double seconds);

signals :
	void cleanupSignal();
	void imageReadySignal(cv::Mat image);
	void reportSignal(MessageLevel level, const QString& string);
	// nothing cool, just calibrate according to video/image. Signal when result is ready
	void calibrationResult(int errorcode, cv::Mat calibration);
	void finishedSignal();
	void inputChangedSignal(const QString& str);

public slots:
	void Report(MessageLevel level, const QString & message);

public:

	//load
	ExtractionWorker * GetWorker() { return &_worker; }

	// load images
	bool Load(const QString & str);

	void timerEvent(QTimerEvent * ev);

	void ShowCurrent();
	FrameProcessor();
	~FrameProcessor();
	void Pause();
	void Stop();
};