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
};

#include "opencv2\features2d.hpp"

class CaptureVideoFrame : public QObject
{
	Q_OBJECT

protected:

	int _mode = 0;

	std::vector<cv::Point2f> _currentPoints;
	cv::Ptr<cv::FastFeatureDetector> _ffd;
	//QColorcolor = np.random.randint(0, 255, (100, 3))
	QBasicTimer _timer;
	QScopedPointer<cv::VideoCapture> _capture;
	int _frameH;
	int _frameW;
	int _fps;
	int _numFrames;
	std::vector<cv::KeyPoint> _keypoints;
	cv::Mat _frame;
	cv::Scalar _colors;
	
	void Init();
	void Fill(cv::Mat&frame, 
		std::vector<cv::KeyPoint>& corners, 
		cv::Mat& gr, 
		std::vector<cv::Point2f>&points);
	public slots:
	
signals :
	void signalImageReady(const QImage& image);

public:
	//load
	bool Load(const QString & str);
	void timerEvent(QTimerEvent * ev);
	void ShowCurrent();
	~CaptureVideoFrame();
	void setFactors(const QSize& size);
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