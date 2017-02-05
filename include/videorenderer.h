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
	// legal only when mouse is pressed
	QPoint _startingMousePos;

	// scale of the showed image
	float _scale;

	// starting x
	int _offsetx;

	// starting y 
	int _offsety;

	// mouse pressed flag
	bool _mousePressed;

	// last pressed point. Must be always regarding the original image
	//cv::Point2f _lastPoint;

	// selected points
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
	void wheelEvent(QWheelEvent * event);
	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void keyPressEvent(QKeyEvent *ev);
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	~VideoRenderer();
	void SetParameters(CalibrationSet calibration);
};
