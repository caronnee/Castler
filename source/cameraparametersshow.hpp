#pragma once
#ifndef CAMERAPARAMETERSSHOW_HPP
#define CAMERAPARAMETERSSHOW_HPP
#include <QWidget>
#include "ui_cameraparametersshow.h"
#include "opencv2/opencv.hpp"
#include <Qsettings>
#include "imageprovider.h"

class CameraParametersShow : public QWidget {
	Q_OBJECT

public:
	CameraParametersShow(QWidget * parent = Q_NULLPTR);
	~CameraParametersShow();

	void Save(QSettings& calSettings);
	void Load(QSettings& calSettings);
	public slots:

	void PrepareCalibration();

	void SetCalibCamera(cv::Mat camera, int type);
private:
	Ui::CameraParametersShow ui;
signals:
	void calibrationChanged(CalibrationSet);
};

#endif // CAMERAPARAMETERSSHOW_HPP