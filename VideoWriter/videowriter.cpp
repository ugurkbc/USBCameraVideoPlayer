#include "videowriter.h"
#include <Utility/utility.h>
#include <QDebug>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsrc.h>

const QString VideoWriter::PATH = "./records/";
const QString VideoWriter::MEDIA_TYPE = ".mp4";
const QString VideoWriter::APPSRC_NAME = "mysrc";
extern bool GST_INIT; // definition videocapture.cpp

VideoWriter::VideoWriter(QObject *parent) : QObject(parent)
{
    this->moveToThread(&mThread);
    mThread.start();

    if(!GST_INIT)
    {
        gst_init(nullptr, nullptr);
        GST_INIT = true;
    }
}

VideoWriter::~VideoWriter()
{
    if(mPipeline)
    {
        close();
    }

    mThread.quit();
    mThread.wait();
}

bool VideoWriter::play(QString pFileName, int pWidth, int pHeight, double pFPS)
{
    if(!mPipeline)
    {
        if(pFileName.contains(".") || pFileName == "" || pWidth <= 0 || pHeight <= 0, pFPS <= 0)
        {
            qDebug() << "INVALID Input Parameters";
            return false;
        }

        mFileName = pFileName + MEDIA_TYPE;
        mWidth = pWidth;
        mHeight = pHeight;
        mFPS = pFPS;

        int lFPSNum = 0, lFPSDenom = 1;
        Utility::toFraction(pFPS, lFPSNum, lFPSDenom);
        mFPSNum = lFPSNum;
        mFPSDenom = lFPSDenom;

        printVideoInfo();

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

bool VideoWriter::close()
{
    if(!mPipeline) return false;

    bool lFlag = true;

    if(gst_app_src_end_of_stream((GstAppSrc *)mAppSrc) != GST_FLOW_OK)
    {
         qDebug() << "Cannot send EOS to GStreamer pipeline";
    }

    GstBus *lBus;
    lBus = gst_element_get_bus((GstElement *)mPipeline);

    if (lBus)
    {
        GstMessage *lMsg;
        lMsg = gst_bus_timed_pop_filtered(lBus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        if (!lMsg || GST_MESSAGE_TYPE(lMsg) == GST_MESSAGE_ERROR)
        {
            handleMessage();
            qDebug() << "Error during VideoWriter finalization";
        }

        gst_object_unref(lBus);
        gst_message_unref(lMsg);
    }
    else
    {
        qDebug() << "can't get GstBus";
    }

    if(!changeState(GST_STATE_NULL))
    {
        lFlag = false;
    }

    clean();

    return lFlag;
}

QString VideoWriter::createPipeline()
{
    return QString("appsrc name=" + APPSRC_NAME + " ! video/x-raw, format=(string)" + mFormat + ", width=(int)"
                   + QString::number(mWidth)+ ", height=(int)" + QString::number(mHeight) + ", framerate=(fraction)" + QString::number(mFPSNum) + "/"
                   + QString::number(mFPSDenom) + " ! videoconvert ! omxh264enc ! h264parse ! mp4mux ! filesink location=" + mFileName);
}

bool VideoWriter::launchPipeline(QString pPipeline)
{
    GError *lError = nullptr;

    mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), &lError);

    if(!printError(lError)) return false;

    if(!(mAppSrc = gst_bin_get_by_name(GST_BIN(mPipeline), APPSRC_NAME.toStdString().c_str())))
    {
        qDebug() << "Error gst_bin_get_by_name";
        return false;
    }

    g_object_set(G_OBJECT(mAppSrc), "format", GST_FORMAT_TIME, NULL);
    g_object_set(G_OBJECT(mAppSrc), "block", 1, NULL);
    g_object_set(G_OBJECT(mAppSrc), "is-live", 0, NULL);

    if(!changeState(GST_STATE_PLAYING)) return false;

    return true;
}

void VideoWriter::printVideoInfo()
{
    qDebug() << "Video Format: " << mFormat;
    qDebug() << "Video Width: " << mWidth;
    qDebug() << "Video Height: " << mHeight;
    qDebug() << "Video FPS: " << mFPS;
}

bool VideoWriter::changeState(int pState)
{
    if(!mPipeline) return false;

    gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    GstState lCurrentState;

    gst_element_get_state((GstElement *)mPipeline, &lCurrentState, nullptr, 0);

    handleMessage();

    emit onStateChange(lCurrentState);

    return true;
}

void VideoWriter::clean()
{
    while(GST_OBJECT_REFCOUNT_VALUE(mPipeline))
    {
        gst_object_unref(mPipeline);
    }

    mPipeline = nullptr;
    mAppSrc = nullptr;
    mWidth = INVALID;
    mHeight = INVALID;
    mFPS = INVALID;
    mNumFrames = 0;
}

bool VideoWriter::init()
{
    if(!launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        clean();
        return false;
    }

    return true;
}

bool VideoWriter::printError(void *pError)
{
    if(pError != nullptr)
    {
        qDebug() << ((GError *)pError)->message;
        return false;
    }

    return true;
}

void VideoWriter::recording(QImage pFrame)
{
    if(!mAppSrc) return;

    GstClockTime lDuration, lTimeStamp;
    int lSize = pFrame.byteCount();

    lDuration = ((double) 1 / mFPS) * GST_SECOND;
    lTimeStamp = mNumFrames * lDuration;

    GstBuffer *lBuffer = gst_buffer_new_allocate(nullptr, lSize, nullptr);
    GstMapInfo lInfo;
    gst_buffer_map(lBuffer, &lInfo, (GstMapFlags)GST_MAP_READ);
    memcpy(lInfo.data, (guint8*)pFrame.bits(), lSize);
    gst_buffer_unmap(lBuffer, &lInfo);

    GST_BUFFER_DURATION(lBuffer) = lDuration;
    GST_BUFFER_PTS(lBuffer) = lTimeStamp;
    GST_BUFFER_DTS(lBuffer) = lTimeStamp;
    GST_BUFFER_OFFSET(lBuffer) = mNumFrames;

    if (gst_app_src_push_buffer((GstAppSrc *) mAppSrc, lBuffer) == GST_FLOW_OK)
    {
        ++mNumFrames;
    }
}

void VideoWriter::handleMessage()
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
