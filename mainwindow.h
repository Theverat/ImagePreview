#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imagehandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QImage currentImage;
    ImageHandler *imageHandler;
    
    void writePositionSettings();
    void readPositionSettings();
    
private slots:
    void initImageLoaded();
    void displayImageInfo();
    void openFolder();
    void convertImages();
};

#endif // MAINWINDOW_H
