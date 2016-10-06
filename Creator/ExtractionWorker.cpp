﻿#include "ExtractionWorker.hpp"
#include "typedefs.h"

ExtractionWorker::ExtractionWorker()
{
	ImageProcessor::_reporter = this;
	_mode = ModeIdle;
	_chesspoints.clear();
	_foundCoords.clear();
	_provider = NULL;
}

void ExtractionWorker::Cleanup()
{
	_timer.stop();
	_mode = ModeIdle;
	_chesspoints.clear();
	if (_provider)
		delete _provider;
	_provider = NULL;

#define COEF 10
	for (int i = 0; i < _chessH; i++)
	{
		for (int j = 0; j < _chessW; j++)
		{
			_chesspoints.push_back(cv::Point3f(COEF*i, COEF*j, 0));
		}
	}
}

void ExtractionWorker::Terminate()
{
	_mode = 0;
}

void ExtractionWorker::ChangeCalibration(CalibrationSet calibration)
{
	_calibrationSet = calibration;
} 

void ExtractionWorker::SetMode(int mode)
{
	_mode ^= mode;
	if ( _timer.isActive() == false )
		_timer.start(0, this);
}

void ExtractionWorker::Report(MessageLevel level, const QString & str)
{
	emit workerReportSignal(level, str);
}

void ExtractionWorker::OpenSlot(QString str)
{
	Cleanup();
	_provider = CreateProvider(str);
	
	DoAssert(_provider != false && _provider->IsValid());
	processor.Init(_provider);
	_mode = ModeIdle;
	_timer.start(10,this);
}

void ExtractionWorker::timerEvent(QTimerEvent * ev)
{
	if (_timer.timerId() != ev->timerId()|| (_mode <= ModeIdle ))
	{
		// nothing to do
		return;
	}
	// one step
	_timer.stop();
	bool cont = RunExtractionStep(processor);
	if (!cont)
	{
		_timer.start(5, this);
		return;
	}
	if (cont && (_mode & ModeCalibrate))
	{
		// last calibration step

		emit workerReportSignal(MInfo, "Calculating parameters");

		// finish when all images are processed
		if (_mode & ModeCalibrate)
		{
			int nFrames = _foundCoords.size();
			PointsArray2 chesses;

			chesses.resize(nFrames, _chesspoints);
			cv::Mat cameraMatrix, distCoeffs, rvecs, tvecs;
			cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
			cameraMatrix.at<double>(0, 0) = 1;
			distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
			cv::Size size = processor.GetSize();

			int flags = cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5 ;
			bool calibrated = _foundCoords.size() && cv::calibrateCamera(chesses, _foundCoords, size, cameraMatrix, distCoeffs,
				rvecs, tvecs, flags );

			if (calibrated)
			{
				emit workerReportSignal(MInfo, "Calibration successful");
				emit camParametersSignal(cameraMatrix, distCoeffs);
			}
			else
			{
				emit workerReportSignal(MError, "Calibration not successfull");
			}
		}
	}
}

void ExtractionWorker::PreparePair(int start, int modifier)
{
	if (!_provider)
		return;
	cv::Mat m1, m2;
	_provider->NextFrame(m1);
	_provider->NextFrame(m2);
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

void ExtractionWorker::Process()
{
	emit workerReportSignal(MInfo, "Starting thread process");
	// already loaded 
	//_timer.start(5, this);
}

bool ExtractionWorker::RunExtractionStep(ImageProcessor& processor )
{
	cv::Size patternSize(_chessW, _chessH);
	{
		if (!processor.Next())
		{
			//gf_report(MInfo, "No more frames to use");
			return true;
		}

		QString str = QString::asprintf("Processing image %d", _provider->Position());

		emit workerReportSignal(MInfo, str);

		// detect
		bool found = false;
		if (_mode & ModeCalibrate)
		{
			CoordsArray corners;
			processor.PerformCalibration(_chessW,_chessH, corners);
			if (corners.size())
			{
				found = true;
				_foundCoords.push_back(corners);
			}
		}
		
		if (_mode & ModeDetect)
		{
			emit workerReportSignal(MInfo, "Detecting features in next image");
			processor.PerformDetection();
		}

		//////////////////////////////////////////////////////
		//// final for showing image after all preprocessing
		//////////////////////////////////////////////////////

		QImage::Format format = QImage::Format_RGB888;
		if (_mode & ModeGrey)
		{
			format = QImage::Format_Grayscale8;
			processor.UseGrey();
		}

		if (_mode & ModeFeatures)
		{	
			processor.DrawFeatures();
		}
		
		if (_mode & ModeUndistort)
		{
			// show undistorted model according to parameters
			processor.ApplyCalibration(_calibrationSet);
		}

		cv::Mat mat =  processor.GetResult();
		
		if (found)
		{
			QString str = QString::asprintf("Found at %d", _provider->Position());
			emit workerReportSignal(MInfo, str.toStdString().c_str());
			cv::drawChessboardCorners(mat, patternSize, cv::Mat(_foundCoords.back()), found);
		}
		
		// emit result
		emit workerReportSignal(MInfo, "Emitting result");
	
		emit imageProcessed(mat, _provider->Step()/1000);
	}
	return false;

}