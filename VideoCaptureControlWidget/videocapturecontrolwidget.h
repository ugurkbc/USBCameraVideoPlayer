#ifndef VIDEOCAPTURECONTROLWIDGET_H
#define VIDEOCAPTURECONTROLWIDGET_H

#include <QWidget>
class VideoCapture;
class ImageWidget;

namespace Ui {
class VideoCaptureControlWidget;
}

class VideoCaptureControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoCaptureControlWidget(VideoCapture *pVideoCapture, ImageWidget *pImageWidget, QWidget *parent = 0);
    ~VideoCaptureControlWidget();

private:
    void close();

private slots:
    void stateChanged(int pVideoState);

private slots:
    void on_pushButton_video_play_stop_clicked();
    void on_pushButton_video_close_clicked();
private:
    Ui::VideoCaptureControlWidget *ui;
    VideoCapture *mVideoCapture;
    ImageWidget *mImageWidget;
    int mState;
};

#endif // VIDEOCONTROLWIDGET_H
