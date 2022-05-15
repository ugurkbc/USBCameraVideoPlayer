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
    void setDevice(int pDeviceNumber);
    bool pause();
    bool destroy();
    bool play();

private:
    QString createPipeline();
    bool launchPipeline(QString pPipeline);
    void printVideoInfo();
    bool changeState(int pState);
    void clean();
    bool init();

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
    bool mInit = false;

private:
    static const QString PREFIX_DEVICE_PATH;
    static const QString APPSINK_NAME;
    static bool GST_INIT;
    static const int INVALID = -1;
signals:
    void onNewFrame(QImage);
};

#endif // VIDEOCAPTURE_H
