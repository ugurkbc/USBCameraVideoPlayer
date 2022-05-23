#include "videorecordcontrolwidget.h"
#include "ui_videorecordcontrolwidget.h"
#include <imagewidget.h>
#include <VideoWriter/videowriter.h>
#include <QDebug>

enum VideoState{
PENDING_ = 0,
NULL_ = 1,
READY_ = 2,
PAUSED_ = 3,
PLAYING_ = 4
};

VideoRecordControlWidget::VideoRecordControlWidget(VideoWriter *pVideoWriter, ImageWidget *pImageWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoRecordControlWidget),
    mImageWidget(pImageWidget),
    mVideoWriter(pVideoWriter)
{
    ui->setupUi(this);

    connect(mVideoWriter, &VideoWriter::onStateChange, this, &VideoRecordControlWidget::stateChanged, Qt::DirectConnection);
}

VideoRecordControlWidget::~VideoRecordControlWidget()
{
    delete ui;
}

void VideoRecordControlWidget::stateChanged(int pVideoState)
{
    mState = pVideoState;

    if(mState == VideoState::PLAYING_)
    {
        ui->pushButton_play_stop_record->setText("Pause");

        connect(mImageWidget, &ImageWidget::onNewFrame, mVideoWriter, &VideoWriter::recording, Qt::ConnectionType::QueuedConnection);
    }
    else
    {
        ui->pushButton_play_stop_record->setText("Play");

        disconnect(mImageWidget, &ImageWidget::onNewFrame, mVideoWriter, &VideoWriter::recording);
    }
}

void VideoRecordControlWidget::on_pushButton_close_record_clicked()
{
    mVideoWriter->close();
}

void VideoRecordControlWidget::on_pushButton_play_stop_record_clicked()
{
    if(mState == VideoState::PLAYING_)
    {
        mVideoWriter->pause();
    }
    else
    {
        if(!mVideoWriter->play("video", 1280, 960, 7.5))
        {
            qDebug() << "Error Init VideoWriter";
        }
    }
}
