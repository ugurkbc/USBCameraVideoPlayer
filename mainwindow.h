#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class VideoCapture;
class ImageWidget;
class VideoControlWidget;

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
    ImageWidget *mImageWidget;
    VideoControlWidget *mVideoControlWidget;
};

#endif // MAINWINDOW_H
