#include "ExtractionWorker.hpp"
#include "typedefs.h"

ExtractionWorker::ExtractionWorker()
{
	ImageProcessor::_reporter = this;
	_mode = VisualModeStop;
}

void ExtractionWorker::Cleanup()
{
	_mode = VisualModeStop;
	_timer.stop();
	_provider.Clear();
}

void ExtractionWorker::Terminate()
{
	Cleanup();
}

void ExtractionWorker::ChangeCalibration(CalibrationSet calibration)
{
	_calibrationSet = calibration;
} 

void ExtractionWorker::ChangeActionMode(int mode)
{
	if (!(_mode & VisualModePlay))
	{
		//start
		_processor.Create(mode);
	}
	ChangeVisualMode(VisualModePlay);
}

void ExtractionWorker::ChangeVisualMode(int mode)
{
	if (mode == 0)
	{
		_mode = 0;
		return;
	}
	_mode ^= mode;
	if ( _timer.isActive() == false )
		_timer.start(0, this);
}

void ExtractionWorker::RequestNextFrame()
{

}

void ExtractionWorker::Report(MessageLevel level, const QString & str)
{
	emit workerReportSignal(level, str);
}

void ExtractionWorker::PointCallback(cv::Point3f & point, int id)
{
	emit pointDetectedSignal(id, point);
}

void ExtractionWorker::OpenSlot(const QString & str)
{
	if ( _timer.isActive())
		_timer.stop();
	Cleanup();
	 
	_provider.Create(str);
	
	DoAssert( _provider.IsValid() );
	_processor.Init(&_provider);
	_processor._detectorOutput = this ;
	_timer.start(0,this);
}

void ExtractionWorker::timerEvent(QTimerEvent * ev)
{
	if (_timer.timerId() != ev->timerId()|| (_mode == VisualModeStop ))
	{
		// nothing to do
		return;
	}

	if (!_processor.Next())
	{
		_timer.stop();
		emit finished();
		return;
	}
	if (_processor.CleanModifiedFlag() == false)
	{
		// nothing to emit
		return;
	}
	//////////////////////////////////////////////////////
	//// final for showing image after all preprocessing
	//////////////////////////////////////////////////////

	QImage::Format format = QImage::Format_RGB888;
	if (_mode & VisualModeGrey)
	{
		format = QImage::Format_Grayscale8;
		_processor.UseGrey();
	}

	if (_mode & VisualModeFeatures)
	{
		_processor.ApplyFeatureDetector();
		_processor.DrawFeatures();
	}

	if (_mode & VisualModeUndistort)
	{
		// show undistorted model according to parameters
		_processor.ApplyCalibration(_calibrationSet);
	}

	cv::Mat mat = _processor.GetResult();
	//if (found)
	//{
	//	QString str = QString::asprintf("Found at %d", _provider.Get()->Position());
	//	emit workerReportSignal(MInfo, str.toStdString().c_str());
	//	//cv::drawChessboardCorners(mat, patternSize, cv::Mat(), found);
	//}

	// emit result
	emit imageProcessed(mat, _processor.Context() );
}

void ExtractionWorker::PreparePair(int start, int modifier)
{
	if (_provider.IsValid()==false)
		return;
	cv::Mat m1, m2;
	_provider.Next();
	_provider.Frame(m1);

	_provider.Next();
	_provider.Frame(m2);
	if (modifier == CannyModifier)
	{
		cv::Mat blurred;
		cv::blur(m1, blurred, cv::Size(15, 15));
		cv::Canny(blurred, m1, 20, 60, 3);
		
		cv::blur(m2, blurred, cv::Size(15, 15));
		cv::Canny(blurred, m2, 20, 60, 3);
	}
	// applyModifier
	emit imagePairSignal(m1, m2);
}

void ExtractionWorker::ProcessActionDone()
{
	//clean up
	_processor.ActionDone();
}

void ExtractionWorker::ProcessPartialAction(PointsContext context)
{
	_processor.ProcessContext(context);
}

void ExtractionWorker::Process()
{
	emit workerReportSignal(MInfo, "Starting thread process");
}

//bool ExtractionWorker::RunExtractionStep(ImageProcessor& _processor )
//{
//	cv::Size patternSize(_chessW, _chessH);
//	{
//		if (!_processor.Next())
//		{
//			if (_mode & ModeCalibrate)
//			{
//				FinishCalibration();
//			}
//
//			if (_mode & ModeCreate)
//			{
//				_processor.AutoCalibrate();
//				_processor.FinishCreation();
//			}
//			return true;
//		}
//
//		QString str = QString::asprintf("Processing image %d", _provider.Get()->Position());
//
//		emit workerReportSignal(MInfo, str);
//
//		// detect
//		bool found = false;
//		if (_mode & ModeCalibrate)
//		{
//			CoordsArray corners;
//			_processor.PerformCalibration(_chessW,_chessH, corners);
//		}
//		
//		if (_mode & ModeCreate)
//		{
//			_processor.ApplyFeatureDetector();
//			_processor.CreateMatches();
//		}
//		if (_mode & ModeDetect)
//		{
//			emit workerReportSignal(MInfo, "Detecting features in next image");
//			_processor.PerformDetection();
//		}
//

//	}
//	return false;
//}

