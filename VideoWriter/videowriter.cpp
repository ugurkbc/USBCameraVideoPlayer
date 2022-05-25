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
    if(mInit)
    {
        close();
    }

    mThread.quit();
    mThread.wait();
}

bool VideoWriter::play(QString pFileName, int pWidth, int pHeight, double pFPS)
{
    if(!mInit)
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

bool VideoWriter::pause()
{
    if(!changeState(GST_STATE_PAUSED))
    {
        qDebug() << "Pausing Failed";

        return false;
    }

    return true;
}

bool VideoWriter::close()
{
    if(!mPipeline) return false;

    bool lFlag = true;

    handleMessage(mPipeline);

    if(gst_app_src_end_of_stream((GstAppSrc *)mAppSrc) != GST_FLOW_OK)
    {
         qDebug() << "Cannot send EOS to GStreamer pipeline";
    }
    else
    {
        GstBus *lBus;
        lBus = gst_element_get_bus((GstElement *)mPipeline);

        if (lBus)
        {
            GstMessage *lMsg;
            lMsg = gst_bus_timed_pop_filtered(lBus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
            if (!lMsg || GST_MESSAGE_TYPE(lMsg) == GST_MESSAGE_ERROR)
            {
                qDebug() << "Error during VideoWriter finalization";
                handleMessage(mPipeline);
            }
        }
        else
        {
            qDebug() << "can't get GstBus";
        }
    }

    if(!changeState(GST_STATE_NULL))
    {
        qDebug() << "Closing Failed";

        lFlag = false;
    }

    clean();

    return lFlag;
}

bool VideoWriter::changeState(int pState)
{
    if(!mPipeline) return false;

    GstStateChangeReturn lState;

    lState = gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    if (lState == GST_STATE_CHANGE_FAILURE)
    {
        qDebug() << "GST_STATE_CHANGE_FAILURE";
        handleMessage (mPipeline);
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

void VideoWriter::clean()
{
    gst_object_unref(mPipeline);
    gst_object_unref(mAppSrc);

    mWidth = INVALID;
    mHeight = INVALID;
    mFPS = INVALID;
    mInit = false;
    mNumFrames = 0;
}

QString VideoWriter::createPipeline()
{
    return QString("appsrc name=" + APPSRC_NAME + " ! video/x-raw, format=(string)" + mFormat + ", width=(int)"
                   + QString::number(mWidth)+ ", height=(int)" + QString::number(mHeight) + ", framerate=(fraction)" + QString::number(mFPSNum) + "/"
                   + QString::number(mFPSDenom) + " ! videoconvert ! omxh264enc ! h264parse ! mp4mux ! filesink location=" + mFileName);
}

bool VideoWriter::launchPipeline(QString pPipeline)
{
    if(!(mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), nullptr))) return false;

    if(!(mAppSrc = gst_bin_get_by_name(GST_BIN(mPipeline), APPSRC_NAME.toStdString().c_str()))) return false;

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

bool VideoWriter::init()
{
    if(!launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        return false;
    }

    mInit = true;

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

void VideoWriter::handleMessage(void *pPipeline)
{
    if(!pPipeline) return;

    GstBus *lBus;
    GstStreamStatusType lStatus;
    GstElement *lElem = nullptr;

    lBus = gst_element_get_bus((GstElement *)pPipeline);

    while (gst_bus_have_pending(lBus))
    {
        GstMessage *lMsg;
        lMsg = gst_bus_pop(lBus);

        if (!lMsg || !GST_IS_MESSAGE(lMsg))
            continue;

        switch (GST_MESSAGE_TYPE (lMsg))
        {
        case GST_MESSAGE_STATE_CHANGED:
            GstState lOldState,lNewstate, lPendstate;
            gst_message_parse_state_changed(lMsg, &lOldState, &lNewstate, &lPendstate);
            break;
        case GST_MESSAGE_ERROR:
        {
            GError *lErr;
            gchar *lDebug;
            gst_message_parse_error(lMsg, &lErr, &lDebug);
            gchar *lName;
            lName = gst_element_get_name(GST_MESSAGE_SRC (lMsg));
            qDebug() << "Embedded video playback halted; module " << lName << " reported: " << lErr->message;

            gst_element_set_state(GST_ELEMENT(pPipeline), GST_STATE_NULL);
            break;
        }
        case GST_MESSAGE_EOS:
            break;
        case GST_MESSAGE_STREAM_STATUS:
            gst_message_parse_stream_status(lMsg, &lStatus, &lElem);
            break;
        default:
            break;
        }
    }
}
