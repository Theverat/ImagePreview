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
    void wheelEvent(QWheelEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

signals:
    void singleImageDropped(QUrl url);
    void multipleImagesDropped(QList<QUrl> urls);
    void folderDropped(QUrl url);
    void keyLeftPressed();
    void keyRightPressed();
    void mouseWheelZoom(bool forward);
    void rightClick();
    void middleClick();
    void controlSPressed();
    void controlCPressed();
};

#endif // GRAPHICSVIEW_H
