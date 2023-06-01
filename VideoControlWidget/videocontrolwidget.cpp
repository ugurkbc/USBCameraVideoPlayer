#include "videocontrolwidget.h"
#include "ui_videocontrolwidget.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>

enum VideoState{
PENDING_ = 0,
NULL_ = 1,
READY_ = 2,
PAUSED_ = 3,
PLAYING_ = 4
};

VideoControlWidget::VideoControlWidget(VideoCapture *pVideoCapture, ImageWidget *pImageWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControlWidget)
    ,mVideoCapture(pVideoCapture)
    ,mState(NULL_)
    ,mImageWidget(pImageWidget)
{
    ui->setupUi(this);
    connect(mVideoCapture, &VideoCapture::onStateChange, this, &VideoControlWidget::stateChanged, Qt::DirectConnection);
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
        connect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage, Qt::DirectConnection);
    }
    else if(mState == VideoState::PAUSED_)
    {
        ui->pushButton_video_play_stop->setText("Play");
    }
    else if(mState == VideoState::NULL_)
    {
        ui->pushButton_video_play_stop->setText("Play");
        mImageWidget->enableUpdate();
        mImageWidget->fillBlack();
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
            mVideoCapture->play(lNum);
        }
    }
}

void VideoControlWidget::on_pushButton_video_close_clicked()
{
    mImageWidget->disableUpdate();
    disconnect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage);
    mVideoCapture->close();
}
