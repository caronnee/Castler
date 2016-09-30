#include "VideoProvider.h"
#include "debug.h"

const char * VideoProvider::videoExts[] = { ".3gp",".avi",".mpeg",".mp4" };
const int VideoProvider::NImages = sizeof(videoExts) / sizeof(videoExts[0]);

IImageProvider * VideoProvider::create(const QString & source)
{
	bool found = false;
	for (int i = 0; i < NImages; i++)
	{
		if (source.endsWith(videoExts[i]))
		{
			found = true;
			break;
		}
	}
	if (found)
		return new VideoProvider(source);
	return nullptr;
}

VideoProvider::VideoProvider(const QString & source)
{
	_capture.reset(new cv::VideoCapture(source.toStdString()));
	_frameH = _capture->get(CV_CAP_PROP_FRAME_HEIGHT);
	_frameW = _capture->get(CV_CAP_PROP_FRAME_WIDTH);
	_fps = _capture->get(CV_CAP_PROP_FPS);
	_numFrames = _capture->get(CV_CAP_PROP_FRAME_COUNT);
	DoAssert(_capture->isOpened());
}

bool VideoProvider::IsValid()
{
	return _capture->isOpened();
}

bool VideoProvider::NextFrame(cv::Mat& frame)
{
	_capture->read(frame);
	cv::cvtColor(frame, frame, CV_BGR2RGB);
	return Position() != _numFrames;
}

void VideoProvider::SetPosition(const int & position)
{
	if (position < 0 || position > _numFrames)
	{
		_capture->set(cv::CAP_PROP_POS_FRAMES, 0);
	}
	else
	{
		_capture->set(cv::CAP_PROP_POS_FRAMES, position);
	}
}

int VideoProvider::Size()
{
	return _numFrames;
}

int VideoProvider::Position()
{
	return _capture->get(cv::CAP_PROP_POS_FRAMES);
}

void VideoProvider::PreviousFrame(cv::Mat& frame)
{
	int position = Position();
	SetPosition(position - 2);
	NextFrame(frame);
}

double VideoProvider::Step()
{
	return _fps;
}
