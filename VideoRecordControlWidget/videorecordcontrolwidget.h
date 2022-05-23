#ifndef VIDEORECORDCONTROLWIDGET_H
#define VIDEORECORDCONTROLWIDGET_H

#include <QWidget>

class ImageWidget;
class VideoWriter;

namespace Ui {
class VideoRecordControlWidget;
}

class VideoRecordControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoRecordControlWidget(VideoWriter *pVideoWriter, ImageWidget *pImageWidget, QWidget *parent = 0);
    ~VideoRecordControlWidget();

public slots:
    void stateChanged(int pVideoState);
private slots:
    void on_pushButton_close_record_clicked();
    void on_pushButton_play_stop_record_clicked();

private:
    Ui::VideoRecordControlWidget *ui;
    ImageWidget *mImageWidget;
    VideoWriter *mVideoWriter;
    int mState = 1;
    int mWidth = INVALID;
    int mHeight = INVALID;
    int mFPS = INVALID;

private:
    static const int INVALID = -1;
};

#endif // VIDEORECORDCONTROLWIDGET_H
