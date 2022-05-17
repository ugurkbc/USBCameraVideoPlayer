#include "imagewidget.h"
#include <QPainter>
#include <QPaintEvent>

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

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter lPainter(this);
    lPainter.drawImage((rect().bottomRight() - mImage.rect().bottomRight()) / 2, mImage);

    mLocker->unlock();
}

void ImageWidget::newImage(QImage pImage)
{
    mLocker->relock();

    mImage = pImage;

    update();
}
