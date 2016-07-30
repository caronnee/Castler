﻿#include "videorenderer.h"
#include <opencv2/highgui/highgui.hpp>
#include <QPainter>
#include "debug.h"
#include "ReportFunctions.h"

void VideoRenderer::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawImage(0, 0, _img);
	//m_img = QImage();
}

void VideoRenderer::setImage(const QImage & img)
{
#if 0
	if (!_img.isNull()) 
		gf_report( LogHandler::MInfo, "Viewer dropped frame!" );
#endif
	_img = img;
//	DoAssert(_img.size() == size());
	update();
}

VideoRenderer::VideoRenderer(QWidget * parent) : QWidget(parent) {
	setAttribute(Qt::WA_OpaquePaintEvent);
	opencvVideoThread.start();
}

void VideoRenderer::Clean()
{
	if (_capturer)
	{
		delete _capturer;
	}
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

void VideoRenderer::RequestPrevFrame()
{
	_capturer->Request(-2);
}

void VideoRenderer::ShowGreyFrame()
{
	_capturer->SwitchMode(ModeGrey);
}


void VideoRenderer::FeaturesFromFrame()
{
	_capturer->SwitchMode(ModeFeatures);
}

void VideoRenderer::RequestNextFrame()
{
	_capturer->Request(0);
}

void VideoRenderer::Pause()
{
	_capturer->Pause();
}
void VideoRenderer::Stop()
{
	_capturer->Stop();
}

void VideoRenderer::Start(const QString & str, VideoAction action)
{
	Clean();
	_capturer = new FrameProcessor();	
	if (!_capturer->Load(str))
	{
		gf_report(MError, "Unable to load video");
		return;
	}
	// set size to rendering size
	_capturer->SetFactors(this->size());
	// connections
	connect(_capturer, SIGNAL(signalImageReady(const QImage &)), this, SLOT(setImage(const QImage&)));
	//_capturer->moveToThread(&opencvVideoThread);
	//opencvVideoThread.start();
	switch (action)
	{
		case ActionCalibrate:
		{
			_capturer->SwitchMode(ModeFeatures);
			_capturer->StartCalibration();
			break;
		}
		case ActionDetect:
		{
			_capturer->StartDetecting();
			break;
		}
		default:
		{
			ErrorMessage("Action not known");
		}
	}
}
