#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "imageprovider.h"
#include <QThread>
#include "ReportFunctions.h"

struct PointsContext
{
	cv::Point p1, p2;
};

class VideoRenderer : public QWidget {
	Q_OBJECT
		;
private:

	PointsContext _pointsContext;

	cv::Mat _img;
	void paintEvent(QPaintEvent *);
	void SwitchKeys(QKeyEvent *e);
signals:
	void reportSignal(MessageLevel level, const QString & str);
	void setCameraSignal(cv::Mat mat, int type);
	void Finished();
	void setCalibrationSignal(CalibrationSet set);
	void PhaseDoneSignal();
	void PartialActionDoneSignal(PointsContext);

public slots:
	void setImage(cv::Mat img);
	void Report(MessageLevel level, const QString & message);
	void ShowParameters(cv::Mat camera, cv::Mat dist);
	
public:
	void keyPressEvent(QKeyEvent *ev);
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	~VideoRenderer();
	void SetParameters(CalibrationSet calibration);
};
