#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QUrl>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(QGraphicsScene *scene, QWidget *parent = 0);
    GraphicsView(QWidget *parent = 0);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void changeImage(QImage image);
    double getScaleFactor();
    void fitImageInView();
    void autoFit();
    void resetImageScale();
    void showHelp();
    void showText(QString text, QColor color = QColor(255, 255, 255));
    
private:
    QGraphicsPixmapItem *currentImage;
    int prevImageWidth;
    int prevImageHeight;
    double wheelPosition;
    double scaleFactor;
    QGraphicsSimpleTextItem *helpTextItem;
    QPoint dragStart;
    
    void init();
    void zoom(int wheelAngle);
    double calcScaleFactor(double wheelPos);
    double calcWheelPosition(double scaleFac);
    
signals:
    void singleImageDropped(QUrl url);
    void multipleImagesDropped(QList<QUrl> urls);
    void folderDropped(QUrl url);
    void keyLeftPressed();
    void keyRightPressed();
    void controlSPressed();
    void controlCPressed();
    void scaleChanged(double newScale);
    void doubleClicked();
    void dragToFolderEvent();
};

#endif // GRAPHICSVIEW_H
