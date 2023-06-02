#include "videocapture.h"
#include <QDebug>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsink.h>
#include <Utility/utility.h>

const QString VideoCapture::PREFIX_DEVICE_PATH = "/dev/video";
const QString VideoCapture::APPSINK_NAME = "mysink";
const QString VideoCapture::STREAM_FORMAT = "RGB";
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

//    if (!gst_debug_is_active()) {
//        gst_debug_set_active(TRUE);
//        gst_debug_set_default_threshold(GST_LEVEL_DEBUG);
//    }
}

VideoCapture::~VideoCapture()
{
    if(mPipeline)
    {
        close();
    }

    mThread.quit();
    mThread.wait();
}

void VideoCapture::cleanImageBuffer(void *pImageBuffer)
{
    gst_buffer_unref((GstBuffer *) pImageBuffer);
}

void VideoCapture::pause()
{
    if(mPlay){
        if(ERROR == changeState(GST_STATE_PAUSED))
        {
            qDebug() << "Pausing Failed";

            clean();
        }
    }
}

void VideoCapture::close()
{
    mPlay = false;
}

void VideoCapture::play(int pDeviceNumber)
{
    mDevicePath = PREFIX_DEVICE_PATH + QString::number(pDeviceNumber);

    if(!mPipeline)
    {
        if(ERROR == init()) {
           clean();
        }
    }
    else
    {
        if(mPlay){
            if(ERROR == changeState(GST_STATE_PLAYING))
            {
                qDebug() << "Playing Failed";
                clean();
            }
        }
    }
}

QString VideoCapture::createPipeline()
{
    return QString("v4l2src device=" + mDevicePath + " ! videoconvert ! video/x-raw, format=" + STREAM_FORMAT + " ! appsink drop=true name=" +  APPSINK_NAME);
}

int VideoCapture::launchPipeline(QString pPipeline)
{
    GError *lError = nullptr;

    mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), &lError);

    if(ERROR == printError(lError) || !mPipeline)
    {
        return ERROR;
    }

    if(!(mAppSink = gst_bin_get_by_name(GST_BIN(mPipeline), APPSINK_NAME.toStdString().c_str())))
    {
        qDebug() << "Error gst_bin_get_by_name";
        return ERROR;
    }

    if(ERROR == changeState(GST_STATE_PLAYING))
    {
        qDebug() << "Error changeState";
        return ERROR;
    }

    return OK;
}

void VideoCapture::printVideoInfo()
{
    GstPad *lPad = nullptr;

    if(!(lPad = gst_element_get_static_pad((GstElement *)mAppSink, "sink")))
    {
        qDebug() << "Error gst_element_get_static_pad";
        return;
    }

    GstCaps *lCaps = nullptr;

    if(!(lCaps = gst_pad_get_current_caps(lPad)))
    {
        qDebug() << "Error gst_pad_get_current_caps";
        return;
    }

    GstStructure *lStructure = nullptr;

    if(!(lStructure = gst_caps_get_structure(lCaps, 0)))
    {
        qDebug() << "Error gst_caps_get_structure";
        return;
    }

    gst_structure_get_int (lStructure, "width", &mWidth);
    gst_structure_get_int (lStructure, "height", &mHeight);

    int lNum = 0, lDenom=1;
    gst_structure_get_fraction(lStructure, "framerate", &lNum, &lDenom);
    mFPS = (double)lNum / (double)lDenom;

    const gchar* lFormat = gst_structure_get_string(lStructure, "format");

    mFormat = QString(lFormat);

    qDebug() << "Video Format: " << mFormat;
    qDebug() << "Video Width: " << mWidth;
    qDebug() << "Video Height: " << mHeight;
    qDebug() << "Video FPS: " << mFPS;

    gst_object_unref(lPad);
}

void VideoCapture::retrieveFrame()
{
    GstClockTime lTimeOutNanoSecond = 3000000000; // 3 second

    QImage lImage;

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
                    lImage = QImage(lInfo.data, mWidth, mHeight, QImage::Format::Format_RGB888, VideoCapture::cleanImageBuffer, lBuf);

                    emit onNewFrame(lImage);
                }
                gst_buffer_ref(lBuf);
                gst_buffer_unmap(lBuf, &lInfo);
            }

            gst_sample_unref(lSample);
        }
        else
        {
            lImage =  QImage(mWidth, mHeight, QImage::Format::Format_RGB888);

            lImage.fill(Qt::black);

            emit onNewFrame(lImage);
        }
    }

    if(ERROR == changeState(GST_STATE_NULL))
    {
        qDebug() << "Closing Failed";
    }

    clean();
}

int VideoCapture::changeState(int pState)
{
    if(!mPipeline) return ERROR;

    gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    GstState lCurrentState;

    while(true){
        GstStateChangeReturn lStateChange = gst_element_get_state((GstElement *)mPipeline, &lCurrentState, nullptr, GST_CLOCK_TIME_NONE);

        if(lStateChange == GST_STATE_CHANGE_FAILURE)
        {
            qDebug() << "GST_STATE_CHANGE_FAILURE";
            return ERROR;
        }
        else if (lStateChange == GST_STATE_CHANGE_SUCCESS)
        {
            qDebug() << "GST_STATE_CHANGE_SUCCESS";
            break;
        }
        else if (lStateChange == GST_STATE_CHANGE_ASYNC)
        {
            qDebug() << "GST_STATE_CHANGE_ASYNC";
        }
        else if (lStateChange == GST_STATE_CHANGE_NO_PREROLL)
        {
            qDebug() << "GST_STATE_CHANGE_NO_PREROLL";
            break;
        }
        else{
            return ERROR;
        }
    }

    emit onStateChange(lCurrentState);

    return OK;
}

void VideoCapture::clean()
{
    gst_object_unref (mPipeline);
    gst_object_unref (mAppSink);

    mWidth = INVALID;
    mHeight = INVALID;
    mFPS = INVALID;
    mFormat = "";

    mPipeline = nullptr;
    mAppSink = nullptr;
}

int VideoCapture::init()
{
    if(ERROR == launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        return ERROR;
    }

    printVideoInfo();

    if(ERROR == checkStream()){
        changeState(GST_STATE_NULL);
        return ERROR;
    }

    mPlay = true;

    QMetaObject::invokeMethod(this, "retrieveFrame");

    return OK;
}

int VideoCapture::printError(void *pError)
{
    if(pError != nullptr)
    {
        qDebug() << ((GError *)pError)->message;
        return ERROR;
    }

    return OK;
}

int VideoCapture::checkStream()
{
    if(mFormat != STREAM_FORMAT  || mWidth <= 0 || mHeight <= 0 || mFPS <= 0)
    {
        qDebug() << "INVALID Values";
        return ERROR;
    }

    return OK;
}
