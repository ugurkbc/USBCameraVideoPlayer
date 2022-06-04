#include "videocapture.h"
#include <QDebug>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsink.h>

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

bool VideoCapture::pause()
{
    if(!changeState(GST_STATE_PAUSED))
    {
        qDebug() << "Pausing Failed";

        return false;
    }

    return true;
}

void VideoCapture::close()
{
    mPlay = false;

    clean();
}

bool VideoCapture::play(int pDeviceNumber)
{
    mDevicePath = PREFIX_DEVICE_PATH + QString::number(pDeviceNumber);

    if(!mPipeline)
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
    return QString("v4l2src device=" + mDevicePath + " ! videoconvert ! video/x-raw, format=" + STREAM_FORMAT + " ! appsink drop=true name=" +  APPSINK_NAME);
}

bool VideoCapture::launchPipeline(QString pPipeline)
{
    GError *lError = nullptr;

    mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), &lError);

    if(!printError(lError))
    {
        return false;
    }

    if(!(mAppSink = gst_bin_get_by_name(GST_BIN(mPipeline), APPSINK_NAME.toStdString().c_str())))
    {
        qDebug() << "Error gst_bin_get_by_name";
        return false;
    }

    if(!changeState(GST_STATE_PLAYING))
    {
        qDebug() << "Error changeState";
        return false;
    }

    GstPad *lPad = nullptr;

    if(!(lPad = gst_element_get_static_pad((GstElement *)mAppSink, "sink")))
    {
        qDebug() << "Error gst_element_get_static_pad";
        return false;
    }

    GstCaps *lCaps = nullptr;

    if(!(lCaps = gst_pad_get_current_caps(lPad)))
    {
        qDebug() << "Error gst_pad_get_current_caps";
        return false;
    }

    GstStructure *lStructure = nullptr;

    if(!(lStructure = gst_caps_get_structure(lCaps, 0)))
    {
        qDebug() << "Error gst_caps_get_structure";
        return false;
    }

    gst_structure_get_int (lStructure, "width", &mWidth);
    gst_structure_get_int (lStructure, "height", &mHeight);

    int lNum = 0, lDenom=1;
    gst_structure_get_fraction(lStructure, "framerate", &lNum, &lDenom);
    mFPS = (double)lNum / (double)lDenom;

    const gchar* lFormat = gst_structure_get_string(lStructure, "format");

    mFormat = QString(lFormat);

    if(!checkStream()) return false;

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
}

bool VideoCapture::changeState(int pState)
{
    if(!mPipeline) return false;

    GstStateChangeReturn lStateChangeReturn;

    lStateChangeReturn = gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    if (lStateChangeReturn == GST_STATE_CHANGE_FAILURE)
    {
        qDebug() << "GST_STATE_CHANGE_FAILURE";
        handleMessage();
        return false;
    }
    else if(lStateChangeReturn == GST_STATE_CHANGE_ASYNC)
    {
        qDebug() << "GST_STATE_CHANGE_ASYNC";
        lStateChangeReturn = gst_element_get_state((GstElement *)mPipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
    }
    else if(lStateChangeReturn == GST_STATE_CHANGE_NO_PREROLL)
    {
        qDebug() << "GST_STATE_CHANGE_NO_PREROLL";
    }
    else if(lStateChangeReturn == GST_STATE_CHANGE_SUCCESS)
    {
        qDebug() << "GST_STATE_CHANGE_SUCCESS";
    }

    handleMessage();

    GstState lCurrentState;

    lStateChangeReturn = gst_element_get_state((GstElement *)mPipeline, &lCurrentState, nullptr, 0);

    emit onStateChange(lCurrentState);

    return true;
}

void VideoCapture::clean()
{
    if(!changeState(GST_STATE_NULL))
    {
        qDebug() << "Closing Failed";
    }

    if(mPipeline)
    {
        gst_object_unref (mPipeline);
    }

    if(mAppSink)
    {
        gst_object_unref (mAppSink);
    }

    mPipeline = nullptr;
    mAppSink = nullptr;
    mWidth = INVALID;
    mHeight = INVALID;
    mFPS = INVALID;
    mFormat = "";
}

bool VideoCapture::init()
{
    if(!launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        clean();
        return false;
    }

    printVideoInfo();

    mPlay = true;

    QMetaObject::invokeMethod(this, "retrieveFrame", Qt::QueuedConnection);

    return true;
}

bool VideoCapture::printError(void *pError)
{
    if(pError != nullptr)
    {
        qDebug() << ((GError *)pError)->message;
        return false;
    }

    return true;
}

bool VideoCapture::checkStream()
{
    if(mFormat != STREAM_FORMAT  || mWidth <= 0 || mHeight <= 0 || mFPS <= 0)
    {
        qDebug() << "INVALID Values";
        return false;
    }

    return true;
}

void VideoCapture::handleMessage()
{
    if(!mPipeline) return;

    GstBus *lBus;

    lBus = gst_element_get_bus((GstElement *)mPipeline);

    while (gst_bus_have_pending(lBus))
    {
        GstMessage *lMsg;
        GError *lErr;
        gchar *lDebugInfo;

        lMsg = gst_bus_pop(lBus);

        if (!lMsg || !GST_IS_MESSAGE(lMsg))
            continue;

        switch (GST_MESSAGE_TYPE (lMsg))
        {
        case GST_MESSAGE_STATE_CHANGED:
            if (GST_MESSAGE_SRC (lMsg) == GST_OBJECT (mPipeline))
            {
                GstState lOldState, lNewState, lPendingState;
                gst_message_parse_state_changed (lMsg, &lOldState, &lNewState, &lPendingState);
                qDebug() << "Pipeline state changed from " << QString(gst_element_state_get_name (lOldState)) << " to " << QString(gst_element_state_get_name (lNewState)) << ":";
            }
            break;
        case GST_MESSAGE_ERROR:
        {
            gst_message_parse_error (lMsg, &lErr, &lDebugInfo);
            g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (lMsg->src), lErr->message);
            g_printerr ("Debugging information: %s\n", lDebugInfo ? lDebugInfo : "none");
            g_clear_error (&lErr);
            g_free (lDebugInfo);
            gst_element_set_state(GST_ELEMENT(mPipeline), GST_STATE_NULL);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print ("End-Of-Stream reached.\n");
            break;
        default:
            break;
        }
    }
}
