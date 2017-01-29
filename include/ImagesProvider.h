#pragma once
#include "imageprovider.h"

struct FImage
{
	bool loaded;
	QString name;
	cv::Mat image;
};

class ImagesProvider : public IImageProvider
{
	int _pos;
	std::vector<FImage> _images;
public:
	static IImageProvider * create(const QString & source);
	static bool IsSupported(const QString & source);

	ImagesProvider(const QString & source);

	virtual bool End();

	virtual double Step();

	virtual bool IsValid();

	virtual bool Next();
	virtual bool Frame(cv::Mat & frame);
	virtual void PreviousFrame(cv::Mat & frame);
	virtual void SetPosition(const int & position);
	virtual int Position();
	virtual int Count();
	virtual const QString Name()const;
};
