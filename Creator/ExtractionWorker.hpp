#pragma once
#include <QObject>
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "imageprovider.h"
#include <set>

enum CaptureModes
{
	ModeFeatures = 1 << 0,
	ModeGrey = 1 << 1,
	ModeCalibrate = 1 << 2,
	ModeDetect = 1 << 3,
	ModeUndistort = 1 << 4
};

class ExtractionWorker : public QObject, public IReportFunction
{
	Q_OBJECT
		;
	int _mode;
	
	// state in which worker is
	bool _exitting;

	//used calibration
	CalibrationSet _calibrationSet;

public:
	// constructor
	ExtractionWorker();

	void SetMode(const int & mode);

	void Report(MessageLevel level, const QString & str);

signals:
	void workerReportSignal(MessageLevel level, const QString & str);
	void finished();
	void imageProcessed(cv::Mat image);
	void camParametersSignal(cv::Mat camera, cv::Mat distCoeffs);

public slots:
	// Main thread loop
	void Process();
	void Terminate();
	void ChangeCalibration(CalibrationSet );
public:
	void Init(IImageProvider * provider);
private:
	// todo create in the thread
	IImageProvider * _provider;
};

