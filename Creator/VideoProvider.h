#pragma once

#include "imageprovider.h"
#include <QScopedPointer>

class VideoProvider : public IImageProvider
{
	QScopedPointer<cv::VideoCapture> _capture;
	int _frameH;
	int _frameW;
	int _fps;
	int _numFrames;

	static const char * videoExts[];
	static const int NImages;
public:

	static IImageProvider * create(const QString & source);

	VideoProvider(const QString & source);

	virtual bool IsValid();
	virtual bool NextFrame(cv::Mat& frame);
	void SetPosition(const int & position);
	virtual int Size();
	int Position();
	virtual void PreviousFrame(cv::Mat& frame);
	virtual double Step();
};
