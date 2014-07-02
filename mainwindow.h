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
    bool fittedInView;
    bool zoomed;
    
    void displayImageInfo(QImage image, QUrl imageUrl);
    void writePositionSettings();
    void readPositionSettings();
    
private slots:
    void initImageLoaded();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
    void zoom(bool forward);
    void openFolder();
    void convertImages();
};

#endif // MAINWINDOW_H
