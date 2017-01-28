#include "cameraparametersshow.hpp"
#include "debug.h"
#include "imageprovider.h"

CameraParametersShow::CameraParametersShow(QWidget * parent) : QWidget(parent) {
	ui.setupUi(this);
}

CameraParametersShow::~CameraParametersShow() {
	
}

void CameraParametersShow::Save(QSettings& calSettings)
{
	calSettings.setValue("k1", ui.k1->value());
	calSettings.setValue("k2", ui.k2->value());
	calSettings.setValue("k3", ui.k3->value());
	calSettings.setValue("f", ui.focusValue->value());
	calSettings.setValue("px", ui.principalXValue->value());
	calSettings.setValue("py", ui.principalYValue->value());
}

void CameraParametersShow::Load(QSettings& calSettings)
{
	QVariant v;
	v = calSettings.value("k1");
	ui.k1->setValue(v.toDouble());
	v = calSettings.value("k2");
	ui.k2->setValue(v.toDouble());
	v = calSettings.value("k3");
	ui.k3->setValue(v.toDouble());

	v = calSettings.value("f");
	ui.focusValue->setValue(v.toDouble());

	v = calSettings.value("px");
	ui.principalXValue->setValue(v.toDouble());

	v = calSettings.value("py");
	ui.principalYValue->setValue(v.toDouble());

}

void CameraParametersShow::SetCalibCamera(cv::Mat camera, int type)
{
	if (type == 0)
	{
#define CAM_X 3
#define CAM_Y 3
		/*DoAssert(camera.cols == 3);
		DoAssert(camera.rows == 3);
		for (int i = 0; i < CAM_X; i++)
		{
		for (int j = 0; j < CAM_Y; j++)
		{
		float v = camera.at<double>(i, j);
		QVariant val(v);
		ui.cameraMatrix->setItem(i, j, new QTableWidgetItem(val.toString()));
		}
		}*/
		// set focus
		double focusVal = camera.at<double>(0, 0);
		ui.focusValue->setValue(focusVal);
		QVariant val = camera.at<double>(0, 2);
		ui.principalXValue->setValue(val.toDouble());
		val = camera.at<double>(1, 2);
		ui.principalYValue->setValue(val.toDouble());
	}
	else if (type == 1)
	{
		std::vector<double> d(camera);
		// set distortion parameters
		ui.k1->setValue(camera.at<double>(0, 0));
		ui.k2->setValue(camera.at<double>(1, 0));
		ui.k3->setValue(camera.at<double>(4.0));

		// tangential will be zero - we don't care for them
	}
	else
	{
		DoAssert(false);
	}
}
void CameraParametersShow::PrepareCalibration()
{
	CalibrationSet calibration;
	calibration.px = ui.principalXValue->value();
	calibration.py = ui.principalYValue->value();

	// focus
	calibration.fx = ui.focusValue->value();
	calibration.fy = ui.focusValue->value();

	calibration.k1 = ui.k1->value();
	calibration.k2 = ui.k2->value();
	calibration.k3 = ui.k3->value();


#if DEBUG_CAMERA
	std::vector<double> array;
	if (mat.isContinuous()) {
		array.assign((double*)mat.datastart, (double*)mat.dataend);
	}
	else {
		for (int i = 0; i < mat.rows; ++i) {
			array.insert(array.end(), (float*)mat.ptr<uchar>(i), (float*)mat.ptr<uchar>(i) + mat.cols);
		}
	}
#endif
	emit calibrationChanged(calibration);
}
