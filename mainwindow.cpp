#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>
#include <VideoControlWidget/videocontrolwidget.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mVideoCapture = new VideoCapture();
    mImageWidget = new ImageWidget();
    mVideoControlWidget = new VideoControlWidget(mVideoCapture);

    ui->verticalLayout_videoarea->addWidget(mImageWidget);
    ui->verticalLayout_videocontrol->addWidget(mVideoControlWidget, 0, Qt::AlignTop);

    connect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage, Qt::DirectConnection);
    connect(mVideoCapture, &VideoCapture::onStateChange, mVideoControlWidget, &VideoControlWidget::stateChanged, Qt::DirectConnection);
}

MainWindow::~MainWindow()
{
    delete mVideoCapture;
    delete ui;
}
