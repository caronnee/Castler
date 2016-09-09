#ifndef CREATOR_H
#define CREATOR_H

#include <QtWidgets/QWidget>
#include "ui_creator.h"
#include "opencv2/highgui/highgui.hpp"
#include <QShortcut>

class Creator : public QWidget, public IReportFunction
{
	Q_OBJECT

	
	std::vector<QShortcut *> _shortcuts;
public:
    Creator(QWidget *parent = 0);
	~Creator();
	void Report(MessageLevel level, const QString & str);

private slots:
	void SendParameters();
	void EnablePlay();
	void PlayVideo();
	void LoadSettings();
	void LoadModel();
	void SaveSettings();
	void LoadCalibration();
	void RunCalibration();
	void SetCalibCamera(cv::Mat camera, int type);
	void LoadCalibrationImages();
private:
    Ui::CreatorContent ui;
};

#endif // CREATOR_H
