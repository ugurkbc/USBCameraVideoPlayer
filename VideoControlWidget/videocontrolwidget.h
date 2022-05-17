#ifndef VIDEOCONTROLWIDGET_H
#define VIDEOCONTROLWIDGET_H

#include <QWidget>
class VideoCapture;

namespace Ui {
class VideoControlWidget;
}

class VideoControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoControlWidget(VideoCapture *pVideoCapture, QWidget *parent = 0);
    ~VideoControlWidget();

    void stateChanged(int pVideoState);

private slots:
    void on_pushButton_video_play_stop_clicked();

    void on_pushButton_video_close_clicked();

private:
    Ui::VideoControlWidget *ui;
    VideoCapture *mVideoCapture;
    int mState;
};

#endif // VIDEOCONTROLWIDGET_H
