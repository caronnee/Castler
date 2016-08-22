#pragma once

#include <QString>
#include <opencv2/opencv.hpp>

class IImageProvider
{
public:
	virtual bool IsValid() = 0;
	virtual bool NextFrame(cv::Mat & frame)=0;
	virtual void PreviousFrame(cv::Mat & frame)=0;
	virtual void SetPosition(const int & position) = 0;
	virtual int Position() = 0;
	virtual double Step() = 0;
};

IImageProvider * CreateProvider(const QString & source);