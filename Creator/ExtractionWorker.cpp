#include "ExtractionWorker.hpp"
#include "typedefs.h"

ExtractionWorker::ExtractionWorker()
{
	ImageProcessor::_reporter = this;
	_mode = ModeIdle;
	_chesspoints.clear();
	_foundCoords.clear();
	int chessW = 11, chessH = 12;
#define COEF 50
	for (int i = 0; i < chessW; i++)
	{
		for (int j = 0; j < chessH; j++)
		{
			_chesspoints.push_back(cv::Point3f(COEF*i, COEF*j, 0));
		}
	}
}

void ExtractionWorker::Cleanup()
{
	_timer.stop();
}
void ExtractionWorker::Terminate()
{
	_mode = 0;
}

void ExtractionWorker::ChangeCalibration(CalibrationSet calibration)
{
	_calibrationSet = calibration;
} 

void ExtractionWorker::SetMode(const int & mode)
{
	_mode ^= mode;
}

void ExtractionWorker::Report(MessageLevel level, const QString & str)
{
	emit workerReportSignal(level, str);
}

void ExtractionWorker::OpenSlot(QString str, int flags)
{
	_provider = CreateProvider(str);
	SetMode(flags);
	DoAssert(_provider != false && _provider->IsValid());
	processor.Init(_provider);
	_timer.start(0, this);
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

			bool calibrated = cv::calibrateCamera(chesses, _foundCoords, size, cameraMatrix, distCoeffs,
				rvecs, tvecs, cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT);

			if (calibrated)
			{
				emit workerReportSignal(MInfo, "Calibration successful");
				emit camParametersSignal(cameraMatrix, distCoeffs);
			}
			else
			{
				emit workerReportSignal(MInfo, "Calibration not successfull");
			}
		}
	}
}

void ExtractionWorker::Process()
{
	emit workerReportSignal(MInfo, "Starting thread process");
	// already loaded 
	_timer.start(5, this);
}

bool ExtractionWorker::RunExtractionStep(ImageProcessor& processor )
{
	cv::Size patternSize(chessW, chessH);
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
			processor.PerformCalibration(chessW,chessH, corners);
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

		if (_mode & ModeUndistort)
		{
			// show undistorted model according to parameters
			processor.ApplyCalibration(_calibrationSet);
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