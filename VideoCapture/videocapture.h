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
    bool pause();
    void close();
    bool play(int pDeviceNumber);

private:
    QString createPipeline();
    bool launchPipeline(QString pPipeline);
    void printVideoInfo();
    bool changeState(int pState);
    void clean();
    bool init();
    bool printError(void *pError);
    bool checkStream();
    void handleMessage();
    void checkRefCount();
    int mRefCount = 0;

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
