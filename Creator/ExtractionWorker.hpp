#pragma once
#include <QObject>
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "ImageProcessor.h"
#include "imageprovider.h"
#include <set>

enum CaptureModes
{
	ModeIdle = 1, // do nothing
	ModeFeatures = 1 << 1,
	ModeGrey = 1 << 2,
	ModeCalibrate = 1 << 3,
	ModeDetect = 1 << 4,
	ModeUndistort = 1 << 5,
};

#include <QBasicTimer>
#include "typedefs.h"
#include <QTimerEvent>
#include "imageprovider.h"

class ExtractionWorker : public QObject, public IReportFunction
{
	Q_OBJECT
		;
	int _mode;
	int _chessW = 11, _chessH=12;
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

	void SetMode(const int & mode);

	void Report(MessageLevel level, const QString & str);

signals:
	void started(int optimalseconds);
	void workerReportSignal(MessageLevel level, const QString & str);
	void finished();
	void imageProcessed(cv::Mat image, double seconds);
	void camParametersSignal(cv::Mat camera, cv::Mat distCoeffs);

public slots:

// main thread loop
	void Process();
	void OpenSlot(QString str, int flags);
	void Cleanup();
	void Terminate();
	void ChangeCalibration(CalibrationSet );

public:
	void timerEvent(QTimerEvent * ev);
};

