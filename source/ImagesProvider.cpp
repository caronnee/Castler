#include "ImagesProvider.h"
#include <QDirIterator>

#include <Shlwapi.h>
#include <io.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

static QStringList imagesExts = { ".jpg",".png",".jpeg",".tga",".tif", ".tiff" };

#include "Filename.h"

void ImagesProvider::SkipCurrent()
{
	std::string str = Name().toStdString();
	std::string dir = ExtractDirectory(str);
	FILE * f = fopen((dir + ".exclude").c_str(), "a+");
	if (!f)
		return;
	fwrite(str.c_str(), 1, str.size(), f);
	fclose(f);
	_images.erase(_images.begin() + _pos);
	if (_pos == _images.size())
		_pos--;
};
IImageProvider * ImagesProvider::create(const QString & source)
{
	if (PathIsDirectoryA(source.toStdString().c_str()) || IsSupported(source))
		return new ImagesProvider(source);
			
	return nullptr;
}

bool ImagesProvider::IsSupported(const QString & source)
{
	int size = sizeof(imagesExts) / sizeof(imagesExts[0]);
	for (int i = 0; i < size; i++)
	{
		if (source.endsWith(imagesExts[i]))
			return true;
	}
	return false;
}

ImagesProvider::ImagesProvider(const QString & source)
{
	_pos = -1;
	QDirIterator it(source, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		it.next();
		FImage image = { false,it.filePath() };
		image.name = image.name.toLower();
		if ( IsSupported( image.name ))
			_images.push_back(image);
	}
}

bool ImagesProvider::End()
{
	return _pos >= _images.size();
}

double ImagesProvider::Step()
{
	//frame holding two seconds
	return _images.size()/2;
}

bool ImagesProvider::IsValid()
{
	return _images.size() > 0;
}

bool ImagesProvider::Next()
{
	if (_pos +1 >= _images.size())
		return false;

	SetPosition(_pos + 1);
	return true;
}

bool ImagesProvider::Frame(cv::Mat & frame)
{
	if (_pos < 0)
		return false;
	if (_images[_pos].loaded == false)
	{
		cv::Mat frame = cv::imread(_images[_pos].name.toStdString().c_str(), CV_LOAD_IMAGE_COLOR);
		if (frame.empty() == false)
			cv::cvtColor(frame, frame, CV_BGR2RGB);
		_images[_pos].image = frame;
	}
	frame = _images[_pos].image;
	return true;
}

void ImagesProvider::PreviousFrame(cv::Mat & frame)
{
	SetPosition(_pos - 1);
	if (_images[_pos].loaded == false)
	{
		_images[_pos].image = cv::imread(_images[_pos].name.toStdString().c_str(), CV_LOAD_IMAGE_COLOR);
	}
	frame = _images[_pos].image;
}

void ImagesProvider::SetPosition(const int & position)
{
	_pos = position;
}

int ImagesProvider::Position()
{
	return _pos;
}

int ImagesProvider::Count()
{
	return _images.size();
}
#include "Filename.h"

const QString ImagesProvider::Name(const int & pos) const
{
	return _images[pos].name;
}
const QString ImagesProvider::Name() const
{
	return Name(_pos);
}

