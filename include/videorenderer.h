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

	//first click should be only for enabling foxcus
	bool _canAccept;

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

	// selected points
	PointsContext _pointsContext;

	// image to be shown
	cv::Mat _img;

	int _startLine;
	int _matchesMode;

	void paintEvent(QPaintEvent *);

	// keyboard handling
	void SwitchKeys(QKeyEvent *e);

signals:

	void reportSignal(MessageLevel level, const QString & str);
	void setCameraSignal(cv::Mat mat, int type);
	void Finished();
	void setCalibrationSignal(CalibrationSet set);
	void PhaseDoneSignal();
	void PartialActionDoneSignal(PointsContext);

public slots:
	void VideoClear();
	void setImage(cv::Mat img, PointsContext context);
	void Report(MessageLevel level, const QString & message);
	void ShowParameters(cv::Mat camera, cv::Mat dist);
	void SetMatchesState(int state);

public:
	void wheelEvent(QWheelEvent * event);
	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void focusOutEvent(QFocusEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void keyPressEvent(QKeyEvent *ev);
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	~VideoRenderer();
	void SetParameters(CalibrationSet calibration);
};
