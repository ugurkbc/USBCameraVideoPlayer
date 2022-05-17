#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QMutexLocker>
#include <QMutex>


class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
    ~ImageWidget();

private:
    QImage mImage;
    QMutexLocker *mLocker;
    QMutex mMutex;

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void newImage(QImage pImage);
};

#endif // IMAGEWIDGET_H
