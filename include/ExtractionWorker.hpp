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
	
	// time to check for input
	QBasicTimer _timer;

	//used calibration
	CalibrationSet _calibrationSet;

	// provider for the images
	Providers _provider;

	// main class for detection
	ImageProcessor _processor;

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
	void imageProcessed(cv::Mat image, PointsContext context);
	void camParametersSignal(cv::Mat camera, cv::Mat distCoeffs);

public slots:
	// main thread loop
	void PreparePair(int start, int modifier);
	void ProcessActionDone();
	void ProcessPartialAction(PointsContext);
	void Process();
	void OpenSlot(const QString& str);
	void Cleanup();
	void Terminate();
	void ChangeCalibration(CalibrationSet );
	void ChangeActionMode(int mode);
	void ChangeVisualMode(int mode);
	void RequestNextFrame();
public:
	void timerEvent(QTimerEvent * ev);
};

