#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QThread>

class VideoCapture : public QObject
{
    Q_OBJECT
public:
    explicit VideoCapture(QObject *parent = nullptr);
    ~VideoCapture();

public slots:
    void play(int pDeviceNumber);
    void pause();
    void close();

protected:
    virtual QString createPipeline();

private:
    int launchPipeline(QString pPipeline);
    void printVideoInfo();
    int changeState(int pState);
    void clean();
    int init();
    int printError(void *pError);
    int checkStream();
    void checkRefCount();
    void getVideoInfo(void *pSample);
    void getVideoInfo();
    static void cleanImageBuffer(void *pImageBuffer);

private slots:
    void retrieveFrame();

private:
    QThread mThread;
    QString mDevicePath = "";
    void *mAppSink = nullptr;
    void *mPipeline = nullptr;
    int mWidth = INVALID;
    int mHeight = INVALID;
    double mFPS = INVALID;
    QString mFormat = "";
    bool mPlay = false;
    int mState = 0;

private:
    static const QString PREFIX_DEVICE_PATH;
    static const QString APPSINK_NAME;
    static const int INVALID = -1;
    static const QString STREAM_FORMAT;

signals:
    void onNewFrame(QImage);
    void onStateChange(int);
};

#endif // VIDEOCAPTURE_H
