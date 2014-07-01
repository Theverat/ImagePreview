#include "graphicsview.h"
#include <QMimeData>
#include <QDropEvent>
#include <QApplication>

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
}

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    QGraphicsView::dragEnterEvent(event);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) {
    QGraphicsView::dragMoveEvent(event);
}

void GraphicsView::dropEvent(QDropEvent* event) {
    QGraphicsView::dropEvent(event);
    
    if(event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        emit singleImageDropped(urls.at(0));
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event) {
    switch (event->key())
    {
    case Qt::Key_Left:
        emit keyLeftPressed();
        break;
    case Qt::Key_Right:
        emit keyRightPressed();
        break;
    case Qt::Key_S:
        //test if control is pressed as well
        if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
            emit controlSPressed();
        }
    }
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
    if(event->delta() > 0)
    {
        emit mouseWheelZoom(true);
    }
    else
    {
        emit mouseWheelZoom(false);
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        emit rightClick();
    }
    else if(event->button() == Qt::MiddleButton) {
        emit middleClick();
    }
}
