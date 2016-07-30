#ifndef CREATOR_H
#define CREATOR_H

#include <QtWidgets/QWidget>
#include "ui_creator.h"
#include "opencv2/highgui/highgui.hpp"

class Creator : public QWidget
{
	Q_OBJECT

		QString _lastDirectory;
public:
    Creator(QWidget *parent = 0);
	~Creator();
	
private slots:
	void EnablePlay();
	void PlayVideo();
	void SaveSettings();
	void LoadCalibration();
	void RunCalibration();

	void LoadCalibrationImages();
private:
    Ui::CreatorContent ui;
};

#endif // CREATOR_H
