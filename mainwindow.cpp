#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mVideoCapture = new VideoCapture();
    mImageWidget = new ImageWidget();
    this->setCentralWidget(mImageWidget);
    connect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage, Qt::DirectConnection);
    mVideoCapture->setDevice(0);
    mVideoCapture->play();
}

MainWindow::~MainWindow()
{
    delete mVideoCapture;
    delete ui;
}
