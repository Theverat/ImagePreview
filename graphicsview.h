#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QUrl>
#include <QtPrintSupport/QPrinter>

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
    void mouseReleaseEvent(QMouseEvent* event);
    void changeImage(QImage image);
    void changeImage(QMovie *gif, QImage firstFrame);
    double getScaleFactor();
    void autoFit();
    void showHelp();
    void showText(QString text, QColor color = QColor(255, 255, 255));
    
private:
    QGraphicsPixmapItem *currentImage;
    int prevImageWidth;
    int prevImageHeight;
    double wheelPosition;
    double scaleFactor;
    QGraphicsSimpleTextItem *helpTextItem;
    
    void init();
    void zoom(int wheelAngle);
    void setScale();
    void choosePixmapTransform();
    double calcScaleFactor(double wheelPos);
    double calcWheelPosition(double scaleFac);
    
public slots:
    void zoom(double scale);
    void fitImageInView();
    void resetImageScale();
    
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
    void deletePressed();
    void rotatePressed();
    
private slots:
    void printPreview(QPrinter *printer);
};

#endif // GRAPHICSVIEW_H
