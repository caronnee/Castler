#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "imageprovider.h"
#include <QThread>
#include "ReportFunctions.h"

class VideoRenderer : public QWidget {
	Q_OBJECT
		;
private:

	cv::Mat _img;
	void paintEvent(QPaintEvent *);

signals:
	void reportSignal(MessageLevel level, const QString & str);
	void setCameraSignal(cv::Mat mat, int type);
	void Finished();
	void setCalibrationSignal(CalibrationSet set);

public slots:
	void setImage(cv::Mat img);
	void Report(MessageLevel level, const QString & message);
	void ShowParameters(cv::Mat camera, cv::Mat dist);
	
public:
	
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	~VideoRenderer();
	void SetParameters(CalibrationSet calibration);
};
