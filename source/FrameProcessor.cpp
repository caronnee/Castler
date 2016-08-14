#include "FrameProcessor.h"
#include <QTimer>
#include <QTimerEvent>
#include "debug.h"
#include "loghandler.h"

void FrameProcessor::StartDetecting()
{
	_timer.start(_seconds, this);
	_thread.start();
}

bool FrameProcessor::Load(const QString & str)
{
	_provider = CreateProvider(str);
	
	if (_provider == false || _provider->IsValid() == false )
		return false;

	_worker.Init(_provider);
	_seconds = _provider->Step() / 1000;
	return true;
}

void FrameProcessor::StartCalibration()
{
	if (_timer.isActive())
	{
		emit reportSignal(MInfo,"Restarting thread");
		Stop();// restart
	}
	else
	{
		emit reportSignal(MInfo, "Starting thread");
	}

	SwitchMode(ModeCalibrate | ModeFeatures);
	_timer.start(_seconds, this);
	_thread.start();
}

void FrameProcessor::timerEvent(QTimerEvent * ev) {
	
	if (ev->timerId() != _timer.timerId()) 
		return;
	ShowCurrent();
}

void FrameProcessor::ShowCurrent()
{
	if (_images.empty())
		return;
	cv::Mat image = _images.top();
	emit imageReadySignal(image);
	_images.pop();
}

void FrameProcessor::ThreadStopped()
{
	emit reportSignal(MInfo, "Thread stopped");
}

void FrameProcessor::Report(MessageLevel level, const QString & message)
{
	emit reportSignal(level, message.toStdString().c_str());
}

FrameProcessor::FrameProcessor()
{
	_provider = NULL;
	// connects
	connect(&_worker, SIGNAL(workerReportSignal(MessageLevel, const QString &)), this, SLOT(Report(MessageLevel, const QString &)));
	
	connect(&_thread, SIGNAL(started()), &_worker, SLOT(Process()));
	connect(&_thread, SIGNAL(finished()), this, SLOT(ThreadStopped()));
	connect(&_worker, SIGNAL(finished()), &_thread, SLOT(quit()));
	
	connect(&_worker, SIGNAL(imageProcessed(cv::Mat)), this, SLOT( ImageReported( cv::Mat )));
	_worker.moveToThread(&_thread);
}

void FrameProcessor::ImageReported(cv::Mat image)
{
	_images.push(image);
}

FrameProcessor::~FrameProcessor()
{
	_timer.stop();
	DoAssert(!_timer.isActive());
}

void FrameProcessor::Pause()
{
	if (!_timer.isActive())
		_timer.start(_seconds, this);
	else
	{
		_timer.stop();
		DoAssert(!_timer.isActive());
	}
}

void FrameProcessor::Stop()
{
	_timer.stop();
	_thread.exit(0);
	_provider->SetPosition(0);
	std::stack<cv::Mat> empty;
	std::swap(_images, empty);
}

void FrameProcessor::Request(int frames)
{
	if (_timer.isActive() || _thread.isRunning())
	{
		reportSignal(MError, "Waiting for thread");
		return;
	}
	_provider->SetPosition(_provider->Position() + frames);
	ShowCurrent();
}

void FrameProcessor::SwitchMode(int flag)
{
	_worker.SetMode(flag);
}

