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
    explicit ImageWidget(QWidget *parent = nullptr);
    ~ImageWidget();

    void fillBlack();
    void disableUpdate();
    void enableUpdate();

private:
    QImage mImage;
    QMutexLocker *mLocker;
    QMutex mMutex;

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void newImage(QImage pImage);

signals:
    void onNewFrame(QImage);
};

#endif // IMAGEWIDGET_H
