#include "videorecordcontrolwidget.h"
#include "ui_videorecordcontrolwidget.h"
#include <imagewidget.h>
#include <VideoWriter/videowriter.h>
#include <QDebug>
#include <Utility/utility.h>

VideoRecordControlWidget::VideoRecordControlWidget(VideoWriter *pVideoWriter, ImageWidget *pImageWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoRecordControlWidget),
    mImageWidget(pImageWidget),
    mVideoWriter(pVideoWriter)
{
    ui->setupUi(this);

    connect(mVideoWriter, &VideoWriter::onStateChange, this, &VideoRecordControlWidget::stateChanged, Qt::QueuedConnection);
}

VideoRecordControlWidget::~VideoRecordControlWidget()
{
    delete ui;
}

void VideoRecordControlWidget::stateChanged(int pVideoState)
{
    mState = pVideoState;

    if(mState == VideoState::READY_)
    {
        ui->pushButton_play_stop_record->setEnabled(false);

        connect(mImageWidget, &ImageWidget::onNewFrame, mVideoWriter, &VideoWriter::recording, Qt::ConnectionType::QueuedConnection);
    }
    else
    {
        ui->pushButton_play_stop_record->setEnabled(true);
    }
}

void VideoRecordControlWidget::on_pushButton_close_record_clicked()
{
    disconnect(mImageWidget, &ImageWidget::onNewFrame, mVideoWriter, &VideoWriter::recording);
    QMetaObject::invokeMethod(mVideoWriter, "close");
}

void VideoRecordControlWidget::on_pushButton_play_stop_record_clicked()
{
    QMetaObject::invokeMethod(mVideoWriter, "play", Qt::QueuedConnection, Q_ARG(QString, ui->lineEdit_file_name->text()), Q_ARG(int, 1280), Q_ARG(int, 960), Q_ARG(double,  7.5));
}
