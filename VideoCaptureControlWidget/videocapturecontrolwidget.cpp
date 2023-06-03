#include "videocapturecontrolwidget.h"
#include "ui_videocapturecontrolwidget.h"
#include <VideoCapture/videocapture.h>
#include <imagewidget.h>
#include <Utility/utility.h>


VideoCaptureControlWidget::VideoCaptureControlWidget(VideoCapture *pVideoCapture, ImageWidget *pImageWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoCaptureControlWidget)
    ,mVideoCapture(pVideoCapture)
    ,mState(NULL_)
    ,mImageWidget(pImageWidget)
{
    ui->setupUi(this);
    connect(mVideoCapture, &VideoCapture::onNewFrame, mImageWidget, &ImageWidget::newImage, Qt::DirectConnection);
    connect(mVideoCapture, &VideoCapture::onStateChange, this, &VideoCaptureControlWidget::stateChanged);
}

VideoCaptureControlWidget::~VideoCaptureControlWidget()
{
    delete ui;
}

void VideoCaptureControlWidget::close()
{
    mVideoCapture->close();
}

void VideoCaptureControlWidget::stateChanged(int pVideoState)
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

void VideoCaptureControlWidget::on_pushButton_video_play_stop_clicked()
{
    if(mState == VideoState::PLAYING_)
    {
        mVideoCapture->pause();
    }
    else
    {
        bool lFlag;

        int lNum =  ui->lineEdit_device_number->text().toInt(&lFlag);

        if(lFlag)
        {
            mVideoCapture->play(lNum);
        }
    }
}

void VideoCaptureControlWidget::on_pushButton_video_close_clicked()
{
    close();
}
