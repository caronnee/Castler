#pragma once

#include <QString>
#include<QObject>
#include "opencv2/highgui/highgui.hpp"
#include <QBasicTimer>
#include <opencv2\core\core.hpp>


enum CaptureModes
{
	ModeNormal = 0,
	ModeFeatures = 1,
	ModeGrey = 2,
	ModeCalibrate = 4
};

#include "opencv2\features2d.hpp"
#include "imageprovider.h"

class FrameProcessor : public QObject
{
	Q_OBJECT

protected:
	cv::Mat _result;
	IImageProvider * _provider;
	int _mode = 0;

	std::vector<cv::Point2f> _currentPoints;
	cv::Ptr<cv::FastFeatureDetector> _ffd;
	//QColorcolor = np.random.randint(0, 255, (100, 3))
	QBasicTimer _timer;
	
	std::vector<cv::KeyPoint> _keypoints;
	cv::Mat _frame;
	cv::Scalar _colors;
	
	std::vector<cv::Point2f> _chesspoints;

	void Fill(cv::Mat&frame, 
		std::vector<cv::KeyPoint>& corners, 
		cv::Mat& gr, 
		std::vector<cv::Point2f>&points);

public slots:
	
signals :
	
	void signalImageReady(const QImage& image);

	// nothing cool, just calibrate according to video/image. Signal when result is ready
	void calibrationResult(int errorcode, cv::Mat calibration);
public:
	//load
	void StartDetecting();
	void StartCalibration();
	bool Load(const QString & str);

	void timerEvent(QTimerEvent * ev);
	void ShowCurrent();
	FrameProcessor();
	~FrameProcessor();
	void SetFactors(const QSize& size);
	void Pause();
	void Stop();
	void Request( int frames);

private:
	int _xSize;
	int _ySize;
	cv::Mat _gr;
	int _seconds;
public:
	void SwitchMode(int flag);
};