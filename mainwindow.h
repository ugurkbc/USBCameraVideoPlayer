#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class VideoCapture;
class ImageWidget;
class VideoControlWidget;
class VideoRecordControlWidget;
class VideoWriter;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    VideoCapture *mVideoCapture;
    VideoWriter *mVideoWriter;
    ImageWidget *mImageWidget;
    VideoControlWidget *mVideoCaptureControlWidget;
    VideoRecordControlWidget *mVideoRecordControlWidget;
};

#endif // MAINWINDOW_H
