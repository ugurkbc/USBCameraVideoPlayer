#include "imagewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <VideoCapture/videocapture.h>

ImageWidget::ImageWidget(VideoCapture *pVideoCapture, QWidget *parent)
    : QWidget(parent)
    , mVideoCapture(pVideoCapture)
{
    mLocker = new QMutexLocker(&mMutex);

    connect(mVideoCapture, &VideoCapture::onNewFrame, this, &ImageWidget::newImage, Qt::DirectConnection);
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

    emit onNewFrame(mImage);

    mLocker->unlock();
}

void ImageWidget::newImage(QImage pImage)
{
    mLocker->relock();

    mImage = pImage;

    update();
}
