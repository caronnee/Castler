#ifndef CREATOR_H
#define CREATOR_H

#include <QtWidgets/QWidget>
#include "ui_creator.h"

class Renderer;

class Creator : public QWidget
{
    Q_OBJECT

public:
    Creator(QWidget *parent = 0);
    ~Creator();

private slots:
    void rotateOneStep();

private:
    Ui::CreatorContent ui;
    Renderer *currentGlWidget;
};

#endif // CREATOR_H
