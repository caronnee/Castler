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

class CaptureVideoFrame : public QObject
{
	Q_OBJECT

protected:

	int _mode = 0;

	//QColorcolor = np.random.randint(0, 255, (100, 3))
	QBasicTimer _timer;
	QScopedPointer<cv::VideoCapture> _capture;
	int _frameH;
	int _frameW;
	int _fps;
	int _numFrames;
	cv::Mat _frame,_status;

	// point for optical flow
	std::vector<cv::Point2f> _oldcorners;
	cv::Scalar _colors;
	
	void Fill(cv::Mat&frame, std::vector<cv::Point2f>& corners, cv::Mat& gr);

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