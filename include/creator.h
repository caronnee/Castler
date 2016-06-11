#ifndef CREATOR_H
#define CREATOR_H

#include <QtWidgets/QWidget>
#include "ui_creator.h"
#include "opencv2/highgui/highgui.hpp"

class Creator : public QWidget
{
    Q_OBJECT

public:
    Creator(QWidget *parent = 0);
    ~Creator();

private slots:
	void PlayVideo();
	void SaveSettings();
    void rotateOneStep();
    void reload();

private:
    Ui::CreatorContent ui;
};

#endif // CREATOR_H
