﻿#pragma once
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

enum Modifiers
{
	NoneModifier,
	CannyModifier,
	NModifiers
};

class ExtractionWorker : public QObject, public IReportFunction, public IDetector
{
	Q_OBJECT
		;
	int _mode;

	//TODO  this should be later autodetected
	int _chessW = 7, _chessH=7;
	
	// time to check for input
	QBasicTimer _timer;
	
	// checkpoints to detect for calibration purposes
	std::vector<cv::Point3f> _chesspoints;

	//used calibration
	CalibrationSet _calibrationSet;

	// provider for the images
	Providers _provider;

	// main class for detection
	ImageProcessor _processor;
private:
	
	bool RunExtractionStep( ImageProcessor& processor );

	void FinishCalibration();
 
public:
	// constructor
	ExtractionWorker();

	// main report function 
	void Report(MessageLevel level, const QString & str);
	
	void PointCallback(cv::Point3f & point, int id);
	
signals:
	void pointDetectedSignal(int id, cv::Point3f point);
	void imagePairSignal(cv::Mat left, cv::Mat right);
	void started(int optimalseconds);
	void workerReportSignal(MessageLevel level, const QString & str);
	void finished();
	void imageProcessed(cv::Mat image, double seconds);
	void camParametersSignal(cv::Mat camera, cv::Mat distCoeffs);

public slots:
	void SetMode(int mode);
	// main thread loop
	void PreparePair(int start, int modifier);
	void Process();
	void OpenSlot(const QString& str);
	void Cleanup();
	void Terminate();
	void ChangeCalibration(CalibrationSet );

public:
	void timerEvent(QTimerEvent * ev);
};

