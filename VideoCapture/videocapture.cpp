#include "videocapture.h"
#include <QDebug>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsink.h>

const QString VideoCapture::PREFIX_DEVICE_PATH = "/dev/video";
const QString VideoCapture::APPSINK_NAME = "mysink";
bool GST_INIT = false;

VideoCapture::VideoCapture(QObject *parent) : QObject(parent)
{
    this->moveToThread(&mThread);

    mThread.start();

    if(!GST_INIT)
    {
        gst_init(nullptr, nullptr);
        GST_INIT = true;
    }
}

VideoCapture::~VideoCapture()
{
    if(mInit)
    {
        close();
    }

    mThread.quit();
    mThread.wait();
}

bool VideoCapture::pause()
{
    if(!changeState(GST_STATE_PAUSED))
    {
        qDebug() << "Pausing Failed";

        return false;
    }

    return true;
}

bool VideoCapture::close()
{
    bool lFlag = true;

    if(!changeState(GST_STATE_NULL))
    {
        qDebug() << "Closing Failed";

        lFlag = false;
    }

    mPlay = false;

    return lFlag;
}

bool VideoCapture::play(int pDeviceNumber)
{
    mDevicePath = PREFIX_DEVICE_PATH + QString::number(pDeviceNumber);

    if(!mInit)
    {
        if(!init()) return false;
    }
    else
    {
        if(!changeState(GST_STATE_PLAYING))
        {
            qDebug() << "Playing Failed";

            return false;
        }
    }

    return true;
}

QString VideoCapture::createPipeline()
{
    return QString("v4l2src device=" + mDevicePath + " ! videoconvert ! video/x-raw, format=RGB ! appsink drop=true name=" +  APPSINK_NAME);
}

bool VideoCapture::launchPipeline(QString pPipeline)
{
    if(!(mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), nullptr))) return false;

    if(!(mAppSink = gst_bin_get_by_name(GST_BIN(mPipeline), APPSINK_NAME.toStdString().c_str()))) return false;

    if(!changeState(GST_STATE_PLAYING)) return false;

    GstPad *lPad = gst_element_get_static_pad((GstElement *)mAppSink, "sink");

    GstCaps *lCaps = gst_pad_get_current_caps(lPad);

    GstStructure *lStructure = gst_caps_get_structure(lCaps, 0);

    gst_structure_get_int (lStructure, "width", &mWidth);
    gst_structure_get_int (lStructure, "height", &mHeight);

    int lNum = 0, lDenom=1;
    gst_structure_get_fraction(lStructure, "framerate", &lNum, &lDenom);
    mFPS = (double)lNum / (double)lDenom;

    const gchar* lFormat = gst_structure_get_string(lStructure, "format");

    mFormat = QString(lFormat);

    return true;
}

void VideoCapture::printVideoInfo()
{
    qDebug() << "Video Format: " << mFormat;
    qDebug() << "Video Width: " << mWidth;
    qDebug() << "Video Height: " << mHeight;
    qDebug() << "Video FPS: " << mFPS;
}

void VideoCapture::retrieveFrame()
{
    GstClockTime lTimeOutNanoSecond = 3000000000; // 3 second

    while(mPlay)
    {
        GstSample *lSample = gst_app_sink_try_pull_sample(GST_APP_SINK(mAppSink), lTimeOutNanoSecond);

        if(lSample)
        {
            GstBuffer *lBuf = gst_sample_get_buffer(lSample);

            if(lBuf)
            {
                GstMapInfo lInfo = {};

                if (gst_buffer_map(lBuf, &lInfo, GST_MAP_READ))
                {
                    emit onNewFrame(QImage(lInfo.data, mWidth, mHeight, QImage::Format::Format_RGB888));
                }

                gst_buffer_unmap(lBuf, &lInfo);
            }

            gst_sample_unref(lSample);
        }
        else
        {
            QImage lImage(mWidth, mHeight, QImage::Format::Format_RGB888);

            lImage.fill(Qt::black);

            emit onNewFrame(lImage);
        }
    }

    clean();
}

bool VideoCapture::changeState(int pState)
{
    if(!mPipeline) return false;

    GstStateChangeReturn lState;

    lState = gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    if (lState == GST_STATE_CHANGE_FAILURE)
    {
        qDebug() << "GST_STATE_CHANGE_FAILURE";
        gst_object_unref (mPipeline);
        return false;
    }

    GstClockTime lTimeOutNanoSecond = 3000000000; // 3 second

    if (gst_element_get_state ((GstElement *)mPipeline, NULL, NULL, lTimeOutNanoSecond) == GST_STATE_CHANGE_FAILURE) {
       qDebug() << "Failed to go into the state";
       return false;
     }

    emit onStateChange(pState);

    return true;
}

void VideoCapture::clean()
{
    gst_object_unref (mPipeline);
    gst_object_unref (mAppSink);

    mWidth = INVALID;
    mHeight = INVALID;
    mFPS = INVALID;
    mFormat = "";
    mInit = false;
}

bool VideoCapture::init()
{
    if(!launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        return false;
    }

    printVideoInfo();

    mInit = true;
    mPlay = true;

    QMetaObject::invokeMethod(this, "retrieveFrame", Qt::QueuedConnection);

    return true;
}
