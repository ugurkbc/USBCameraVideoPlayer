#include "imagewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <VideoCapture/videocapture.h>
#include <QDebug>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
    , mLocker(&mMutex)
{
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter lPainter(this);

    lPainter.drawImage((rect().bottomRight() - mImage.rect().bottomRight()) / 2, mImage);

    emit onNewFrame(mImage);

    mLocker.unlock();
}

void ImageWidget::newImage(QImage pImage)
{
    mLocker.relock();

    mImage = pImage;

    update();
}
