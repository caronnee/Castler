#pragma once
#include <QObject>
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "ImageProcessor.h"
#include "imageprovider.h"
#include <set>

#include <QBasicTimer>
#include "typedefs.h"
#include <QTimerEvent>
#include "imageprovider.h"

class ExtractionWorker : public QObject, public IReportFunction
{
	Q_OBJECT
		;
	int _mode;
	int _chessW = 7, _chessH=7;
	// time to check for input
	QBasicTimer _timer;

	CoordsArray2 _foundCoords;

	std::vector<cv::Point3f> _chesspoints;

	//used calibration
	CalibrationSet _calibrationSet;

	// provider for the images
	IImageProvider * _provider;

	ImageProcessor processor;
private:
	bool RunExtractionStep( ImageProcessor& processor );

public:
	// constructor
	ExtractionWorker();


	void Report(MessageLevel level, const QString & str);

signals:
	void imagePairSignal(cv::Mat left, cv::Mat right);
	void started(int optimalseconds);
	void workerReportSignal(MessageLevel level, const QString & str);
	void finished();
	void imageProcessed(cv::Mat image, double seconds);
	void camParametersSignal(cv::Mat camera, cv::Mat distCoeffs);

public slots:
	void SetMode(int mode);
	// main thread loop
	void PreparePair(int start);
	void Process();
	void OpenSlot(QString str);
	void Cleanup();
	void Terminate();
	void ChangeCalibration(CalibrationSet );

public:
	void timerEvent(QTimerEvent * ev);
};

