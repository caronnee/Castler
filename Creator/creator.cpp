#include "creator.h"
#include "renderer.h"
#include <QtWidgets>

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

/// connecters

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
    timer->start(20);

    connect(ui.loadButton, SIGNAL(clicked(void)), this, SLOT(reload()));
}

Creator::~Creator()
{

}

void Creator::reload()
{
  QString name = ui.modelName->text();
  
}

void Creator::rotateOneStep()
{
    ui.renderWidget->rotateBy(+2 * 16, +2 * 16, -1 * 16);
}