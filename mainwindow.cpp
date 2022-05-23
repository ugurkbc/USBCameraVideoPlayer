#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>
#include <VideoControlWidget/videocontrolwidget.h>
#include <VideoRecordControlWidget/videorecordcontrolwidget.h>
#include <VideoWriter/videowriter.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mVideoCapture = new VideoCapture();
    mVideoWriter = new VideoWriter();
    mImageWidget = new ImageWidget(mVideoCapture);
    mVideoControlWidget = new VideoControlWidget(mVideoCapture);
    mVideoRecordControlWidget = new VideoRecordControlWidget(mVideoWriter, mImageWidget);

    ui->verticalLayout_videoarea->addWidget(mImageWidget);
    ui->verticalLayout_videocontrol->addWidget(mVideoControlWidget, 1, Qt::AlignTop);
    ui->verticalLayout_videocontrol->addWidget(mVideoRecordControlWidget, 1, Qt::AlignTop);
}

MainWindow::~MainWindow()
{
    delete mVideoCapture;
    delete mVideoWriter;
    delete ui;
}
