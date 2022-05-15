#include "imagewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
    , mLocker(&mMutex)
{

}

ImageWidget::~ImageWidget()
{
    mLocker.unlock();
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter lPainter(this);
    lPainter.drawImage((rect().bottomRight() - mImage.rect().bottomRight()) / 2, mImage);

    mLocker.unlock();
}

void ImageWidget::newImage(QImage pImage)
{
    mLocker.relock();

    mImage = pImage;

    update();
}
