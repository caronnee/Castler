#include "ExtractionWorker.hpp"
#include "typedefs.h"

ExtractionWorker::ExtractionWorker()
{
	_exitting = false;
	_mode = 0;
}

void ExtractionWorker::Terminate()
{
	_exitting = true;
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

#include "ImageProcessor.h"

void ExtractionWorker::Init(IImageProvider * provider)
{
	_provider = provider;
}

void ExtractionWorker::Process()
{
	emit workerReportSignal(MInfo, "Starting thread process");

	ImageProcessor processor(this,_provider);
	if (!processor.Init())
	{
		emit workerReportSignal(MError, "Unable to prepare");
		emit finished();
		return;
	}

	bool working = true;
	int chessW = 11, chessH = 12;
	std::vector<cv::Point3f> _chesspoints;

#define COEF 50
	for (int i = 0; i < chessW; i++)
	{
		for (int j = 0; j < chessH; j++)
		{
			_chesspoints.push_back(cv::Point3f(COEF*i, COEF*j,0));
		}
	}

	CoordsArray2 foundCoords;

	int index = 0;
	cv::Size patternSize(chessW, chessH);
	while (working)
	{
		if (_exitting)
			return;

		if (!processor.Next())
		{
			//gf_report(MInfo, "No more frames to use");
			break;
		}

		QString str = QString::asprintf("Processing image %d", index++);

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
				foundCoords.push_back(corners);
			}
		}
		
		if (_mode & ModeDetect)
		{
			emit workerReportSignal(MInfo, "Detecting features in next image");
			processor.PerformDetection();
		}


		if (_mode & ModeCalibrate)
		{

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
			working &= processor.DrawFeatures();
		}
		
		cv::Mat mat =  processor.GetResult();
		
		if (found)
		{
			QString str = QString::asprintf("Found at %d", index);
			emit workerReportSignal(MInfo, str.toStdString().c_str());
			cv::drawChessboardCorners(mat, patternSize, cv::Mat(foundCoords.back()), found);
		}
		
		// emit result
		emit workerReportSignal(MInfo, "Emitting result");
	
		emit imageProcessed(mat);
	}

	if (_exitting)
		return;
	emit workerReportSignal(MInfo, "Calculating parameters");

	// finish when all images are processed
	if (_mode & ModeCalibrate)
	{
		int nFrames = foundCoords.size();
		PointsArray2 chesses;

		chesses.resize(nFrames, _chesspoints);
		cv::Mat cameraMatrix, distCoeffs, rvecs, tvecs;
		cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
		cameraMatrix.at<double>(0, 0) = 1;
		distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
		cv::Size size = processor.GetSize();

		bool calibrated = cv::calibrateCamera(chesses, foundCoords, size, cameraMatrix, distCoeffs, 
			rvecs, tvecs, cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST| cv::CALIB_FIX_PRINCIPAL_POINT);

		if (calibrated)
		{
			emit workerReportSignal(MInfo, "Calibration successful");
			emit camParametersSignal(cameraMatrix, distCoeffs);
		}
		else
		{
			emit workerReportSignal(MInfo, "Calibration");
		}

	}	

	// we found all the chessboards;
	emit finished();
}