#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"

void VideoRenderer::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawImage(0, 0, _img);
	//m_img = QImage();
}

void VideoRenderer::setImage(const QImage & img)
{
	if (!_img.isNull()) 
		gf_report( LogHandler::MInfo, "Viewer dropped frame!" );
	_img = img;
	DoAssert(_img.size() == size());
	update();
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	setAttribute(Qt::WA_OpaquePaintEvent);
	opencvVideoThread.start();
}

void VideoRenderer::Clean()
{
	if (_capturer)
		delete _capturer;

	_capturer = NULL;
	if ( opencvVideoThread.isRunning())
	{
		// close all objects
		opencvVideoThread.quit();
		//TODO investigate
		opencvVideoThread.wait();
	}
}

VideoRenderer::~VideoRenderer()
{
	Clean();
}


void VideoRenderer::Load(const QString & str)
{
	Clean();
	_capturer = new CaptureVideoFrame();	
	if (!_capturer->Load(str))
	{
		gf_report(LogHandler::MError, "Unable to load video");
		return;
	}
	// set size to rendering size
	_capturer->setFactors(this->size());
	// connections
	connect(_capturer, SIGNAL(signalImageReady(const QImage &)), this, SLOT(setImage(const QImage&)));
	_capturer->moveToThread(&opencvVideoThread);
	opencvVideoThread.start();
}
