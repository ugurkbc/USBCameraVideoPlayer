#include "videocontrolwidget.h"
#include "ui_videocontrolwidget.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>
#include <Utility/utility.h>


VideoControlWidget::VideoControlWidget(VideoCapture *pVideoCapture, ImageWidget *pImageWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControlWidget)
    ,mVideoCapture(pVideoCapture)
    ,mState(NULL_)
    ,mImageWidget(pImageWidget)
{
    ui->setupUi(this);
    connect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage, Qt::DirectConnection);
    connect(mVideoCapture, &VideoCapture::onStateChange, this, &VideoControlWidget::stateChanged);
}

VideoControlWidget::~VideoControlWidget()
{
    delete ui;
}

void VideoControlWidget::close()
{
    mVideoCapture->close();
}

void VideoControlWidget::stateChanged(int pVideoState)
{
    mState = pVideoState;

    if(mState == VideoState::PLAYING_)
    {
        ui->pushButton_video_play_stop->setText("Pause");

    }
    else if(mState == VideoState::PAUSED_)
    {
        ui->pushButton_video_play_stop->setText("Play");
    }
    else if(mState == VideoState::NULL_)
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
            mVideoCapture->play(lNum);
        }
    }
}

void VideoControlWidget::on_pushButton_video_close_clicked()
{
    close();
}
