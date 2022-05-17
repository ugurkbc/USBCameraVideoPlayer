#include "videocontrolwidget.h"
#include "ui_videocontrolwidget.h"
#include <VideoCapture/videocapture.h>

enum VideoState{
PENDING_ = 0,
NULL_ = 1,
READY_ = 2,
PAUSED_ = 3,
PLAYING_ = 4
};

VideoControlWidget::VideoControlWidget(VideoCapture *pVideoCapture, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControlWidget)
    ,mVideoCapture(pVideoCapture)
    ,mState(NULL_)
{
    ui->setupUi(this);
}

VideoControlWidget::~VideoControlWidget()
{
    delete ui;
}

void VideoControlWidget::stateChanged(int pVideoState)
{
    mState = pVideoState;

    if(mState == VideoState::PLAYING_)
    {
        ui->pushButton_video_play_stop->setText("Pause");
    }
    else
    {
        ui->pushButton_video_play_stop->setText("Play");
    }
}

void VideoControlWidget::on_pushButton_video_play_stop_clicked()
{
    if(mState == VideoState::PLAYING_)
    {
        mVideoCapture->pause();
    }
    else
    {
        bool tFlag;

        int lNum =  ui->lineEdit_device_number->text().toInt(&tFlag);

        if(tFlag)
        {
            mVideoCapture->setDevice(lNum);
            mVideoCapture->play();
        }
    }
}

void VideoControlWidget::on_pushButton_video_close_clicked()
{
    mVideoCapture->close();
}
