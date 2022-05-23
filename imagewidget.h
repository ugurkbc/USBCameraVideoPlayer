#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QMutexLocker>
#include <QMutex>

class VideoCapture;

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(VideoCapture *pVideoCapture, QWidget *parent = nullptr);
    ~ImageWidget();

private:
    QImage mImage;
    QMutexLocker *mLocker;
    QMutex mMutex;
    VideoCapture *mVideoCapture;

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void newImage(QImage pImage);

signals:
    void onNewFrame(QImage);
};

#endif // IMAGEWIDGET_H
