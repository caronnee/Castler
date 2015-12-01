#include "creator.h"
#include "renderer.h"
#include <QtWidgets>

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    QGridLayout *mainLayout = new QGridLayout;

    setLayout(mainLayout);
    currentGlWidget = new Renderer;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
    timer->start(20);
    mainLayout->addWidget(currentGlWidget, 0, 0);

    setWindowTitle(tr("Textures"));
}

Creator::~Creator()
{

}

void Creator::rotateOneStep()
{
    currentGlWidget->rotateBy(+2 * 16, +2 * 16, -1 * 16);
}