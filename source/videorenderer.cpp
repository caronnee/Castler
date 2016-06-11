#include "videorenderer.h"

VideoRenderer::VideoRenderer(QWidget * parent) : QOpenGLWidget(parent) {
	_capture = NULL;
}

#include <QColor>

void VideoRenderer::initializeGL()
{
	initializeOpenGLFunctions();
	qDebug("initializeGL called");
	glClearColor(0.2f, 0.2f, 0.2f,1.f);      // set clear color to black
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, this->width(), this->height(), 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &_texture);

	glDisable(GL_TEXTURE_2D);
}

#include "loghandler.h"

void VideoRenderer::paintGL()
{
	if (!_capture)
	{
		return;
	}

	gf_report(LogHandler::MInfo, "Rendering frame");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, this->width(), this->height(), 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R, _frame->width, _frame->height, 0, GL_R, GL_UNSIGNED_BYTE, _frame->imageData);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 1); glVertex2i(0, this->height());
	glTexCoord2i(0, 0); glVertex2i(0, 0);
	glTexCoord2i(1, 0); glVertex2i(this->width(), 0);
	glTexCoord2i(1, 1); glVertex2i(this->width(), this->height());
	glEnd();
	glFlush();
}

#include "loghandler.h"

void VideoRenderer::Load(const QString& str)
{
	if (_capture)
		cvReleaseCapture(&_capture);

	_capture = cvCaptureFromAVI(str.toLocal8Bit().data());

	if (!cvGrabFrame(_capture)) {              // capture a frame 
		gf_report(LogHandler::MError, "Could not grab a frame\n\7");
		return;
	}
	cvQueryFrame(_capture); // this call is necessary to get correct 
						   // capture properties
	_frameH = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
	_frameW = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH);
	_fps = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FPS);
	_numFrames = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_COUNT);
	_frame = cvQueryFrame(_capture);
	update();
}

VideoRenderer::~VideoRenderer() {
	
}
