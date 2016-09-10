#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "FrameProcessor.h"
#include <QThread>

class VideoRenderer : public QWidget {
	Q_OBJECT
		;
private:

	FrameProcessor _capturer;
	cv::Mat _img;
	void paintEvent(QPaintEvent *);

signals:
	void reportSignal(MessageLevel level, const QString & str);
	void setCameraSignal(cv::Mat mat, int type);
	void Finished();
	void modeChanged(int);
	void setCalibrationSignal(CalibrationSet set);

public slots:
	void ShowGreyFrame();
	void FeaturesFromFrame();
	void setImage(cv::Mat img);
	void Stop();
	void Report(MessageLevel level, const QString & message);
	void NoMoreImages();
	void ShowParameters(cv::Mat camera, cv::Mat dist);
	void Pause();
	void RequestNextFrame();
	void RequestPrevFrame();

public:

	enum VideoAction
	{
		ActionDetect,
		ActionCalibrate,
		NActions
	};

	VideoRenderer(QWidget * parent = Q_NULLPTR);
	~VideoRenderer();

	bool Start(const QString & str, VideoAction action);
//	void SetParameters( CalibrationSet calibration);
	void SetParameters(CalibrationSet calibration);
};
