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

int VideoWriter::play(QString pFileName, int pWidth, int pHeight, double pFPS)
{
    if(!mPipeline)
    {
        if(pFileName.contains(".") || pFileName == "" || pWidth <= 0 || pHeight <= 0, pFPS <= 0)
        {
            qDebug() << "INVALID Input Parameters";
            return ERROR;
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

        if(ERROR == init()) return ERROR;
    }
    else
    {
        if(ERROR == changeState(GST_STATE_PLAYING))
        {
            qDebug() << "Playing Failed";
            return ERROR;
        }
    }

    return OK;
}

void VideoWriter::close()
{
    if(!mPipeline) return;

    if(gst_app_src_end_of_stream((GstAppSrc *)mAppSrc) != GST_FLOW_OK)
    {
         qDebug() << "Cannot send EOS to GStreamer pipeline";
    }

    changeState(GST_STATE_NULL);

    clean();
}

QString VideoWriter::createPipeline()
{
    return QString("appsrc name=" + APPSRC_NAME + " ! video/x-raw, format=(string)" + mFormat + ", width=(int)"
                   + QString::number(mWidth)+ ", height=(int)" + QString::number(mHeight) + ", framerate=(fraction)" + QString::number(mFPSNum) + "/"
                   + QString::number(mFPSDenom) + " ! videoconvert ! omxh264enc ! h264parse ! mp4mux ! filesink location=" + mFileName);
}

int VideoWriter::launchPipeline(QString pPipeline)
{
    GError *lError = nullptr;

    mPipeline = gst_parse_launch(pPipeline.toStdString().c_str(), &lError);

    if(!printError(lError)) return ERROR;

    if(!(mAppSrc = gst_bin_get_by_name(GST_BIN(mPipeline), APPSRC_NAME.toStdString().c_str())))
    {
        qDebug() << "Error gst_bin_get_by_name";
        return ERROR;
    }

    g_object_set(G_OBJECT(mAppSrc), "format", GST_FORMAT_TIME, NULL);
    g_object_set(G_OBJECT(mAppSrc), "block", 1, NULL);
    g_object_set(G_OBJECT(mAppSrc), "is-live", 0, NULL);

    if(ERROR == changeState(GST_STATE_PLAYING)) return ERROR;

    return OK;
}

void VideoWriter::printVideoInfo()
{
    qDebug() << "Video Format: " << mFormat;
    qDebug() << "Video Width: " << mWidth;
    qDebug() << "Video Height: " << mHeight;
    qDebug() << "Video FPS: " << mFPS;
}

int VideoWriter::changeState(int pState)
{
    if(!mPipeline) return ERROR;

    gst_element_set_state(GST_ELEMENT(mPipeline), (GstState)pState);

    GstState lCurrentState;

    gst_element_get_state((GstElement *)mPipeline, &lCurrentState, nullptr, 0);

    emit onStateChange(lCurrentState);

    return OK;
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

int VideoWriter::init()
{
    if(!launchPipeline(createPipeline()))
    {
        qDebug() << "Pipeline Launch Error";
        clean();
        return ERROR;
    }

    return OK;
}

int VideoWriter::printError(void *pError)
{
    if(pError != nullptr)
    {
        qDebug() << ((GError *)pError)->message;
        return ERROR;
    }

    return OK;
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
