#include "FrameProcessor.h"
#include <QTimer>
#include <QTimerEvent>
#include "debug.h"
#include "loghandler.h"

bool FrameProcessor::Load(const QString & str)
{
	if ( !_thread.isRunning())
		_thread.start();
	emit inputChangedSignal(str);
	return true;
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
	// connects
	connect(&_worker, SIGNAL(workerReportSignal(MessageLevel, const QString &)), this, SLOT(Report(MessageLevel, const QString &)));

	_worker.moveToThread(&_thread);

	connect(this, SIGNAL(inputChangedSignal(const &QString)), &_worker, SLOT(OpenSlot(const & QString)));
	connect(&_thread, SIGNAL(started()), &_worker, SLOT(Process()));
	connect(&_thread, SIGNAL(finished()), this, SLOT(ThreadStopped()));
	connect(this, SIGNAL(cleanupSignal()), &_worker, SLOT(Cleanup()));
	connect(&_thread, SIGNAL(finished()), &_worker, SLOT(deleteLater()));
	connect(&_thread, SIGNAL(finished()), &_worker, SLOT(Terminate()));
	connect(&_worker, SIGNAL(imageProcessed(cv::Mat, double)), this, SLOT( ImageReported( cv::Mat,double )));
}

void FrameProcessor::ImageReported(cv::Mat image, double seconds)
{
	_images.push(image);
	if (!_timer.isActive())
		_timer.start(seconds, this);
}

FrameProcessor::~FrameProcessor()
{
	_timer.stop();
	DoAssert(!_timer.isActive());
	emit cleanupSignal();
	_thread.disconnect();
//	_thread.wait();
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
	_worker.Terminate();
	_timer.stop();
	std::stack<cv::Mat> empty;
	std::swap(_images, empty);
}
