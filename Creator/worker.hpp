#pragma once
#include <QObject>
#include "debug.h"
#include "loghandler.h"
#include <opencv2/opencv.hpp>
#include "imageprovider.h"
#include <set>

enum CaptureModes
{
	ModeFeatures = 1,
	ModeGrey = 2,
	ModeCalibrate = 4,
	ModeDetect = 8
};

class Worker : public QObject, public IReportFunction
{
	Q_OBJECT
		;
	int _mode;
	bool _exitting;
public:
	// constructor
	Worker();

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
public:
	void Init(IImageProvider * provider);
private:
	// todo create in the thread
	IImageProvider * _provider;
};

