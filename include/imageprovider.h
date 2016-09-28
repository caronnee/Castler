#pragma once

#include <QString>
#include <QMetaType>
#include <opencv2/opencv.hpp>

enum CaptureModes
{
	ModeIdle = 1, // do nothing
	ModeFeatures = 1 << 1,
	ModeGrey = 1 << 2,
	ModeCalibrate = 1 << 3,
	ModeDetect = 1 << 4,
	ModeUndistort = 1 << 5,
};

struct CalibrationSet
{
	float px, py, k1, k2, k3, fx, fy;
	CalibrationSet()
	{
		px = 0, py = 0, k1 = 0, k2 = 0, k3 = 0, fx = 20, fy = 20;
	}
};

Q_DECLARE_METATYPE(CalibrationSet)

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