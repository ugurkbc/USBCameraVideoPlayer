#include "imagewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <VideoCapture/videocapture.h>
#include <QDebug>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
{
    mLocker = new QMutexLocker(&mMutex);
}

ImageWidget::~ImageWidget()
{
    mLocker->unlock();
    delete mLocker;
}

void ImageWidget::fillBlack()
{
    mImage =  QImage(width(), height(), QImage::Format::Format_RGB888);

    mImage.fill(Qt::black);

    update();
}

void ImageWidget::disableUpdate()
{
    setUpdatesEnabled(false);

    mLocker->unlock();
}

void ImageWidget::enableUpdate()
{
    setUpdatesEnabled(true);
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter lPainter(this);

    lPainter.drawImage((rect().bottomRight() - mImage.rect().bottomRight()) / 2, mImage);

    emit onNewFrame(mImage);
}

void ImageWidget::newImage(QImage pImage)
{
    mLocker->relock();

    mImage = pImage;

    update();
}
