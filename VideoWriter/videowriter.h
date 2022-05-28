#ifndef VIDEOWRITER_H
#define VIDEOWRITER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QImage>

class VideoWriter : public QObject
{
    Q_OBJECT
public:
    explicit VideoWriter(QObject *parent = nullptr);
    ~VideoWriter();

    bool play(QString pFileName, int pWidth, int pHeight, double pFPS);
    bool close();
private:
    QString createPipeline();
    bool launchPipeline(QString pPipeline);
    void printVideoInfo();
    bool changeState(int pState);
    void clean();
    bool init();
    bool printError(void *pError);
    void handleMessage();

public slots:
    void recording(QImage pFrame);

private:
    QThread mThread;
    void *mAppSrc = nullptr;
    void *mPipeline = nullptr;
    QString mFileName = "";
    int mWidth = INVALID;
    int mHeight = INVALID;
    double mFPS = INVALID;
    QString mFormat = "RGB";
    int mNumFrames = 0;
    int mFPSNum = INVALID;
    int mFPSDenom = INVALID;

private:
    static const QString PATH;
    static const QString APPSRC_NAME;
    static const QString MEDIA_TYPE;
    static const int INVALID = -1;

signals:
    void onStateChange(int);
};

#endif // VIDEOWRITER_H
