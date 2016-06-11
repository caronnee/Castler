#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "opencv2/highgui/highgui.hpp"

class VideoRenderer : public QOpenGLWidget, public QOpenGLFunctions {
	Q_OBJECT

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
public:
	VideoRenderer(QWidget * parent = Q_NULLPTR);
	void Load(const QString& str);
	~VideoRenderer();

private:
	CvCapture* _capture;
	int _frameH;
	int _frameW;
	int _fps;
	int _numFrames;
	IplImage* _frame;
	GLuint _texture;
};
