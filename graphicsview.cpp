#include "graphicsview.h"
#include <QMimeData>
#include <QDropEvent>
#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QTextStream>

#include <math.h>

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent) : 
    QGraphicsView(scene, parent)
{
    init();
}

GraphicsView::GraphicsView(QWidget *parent) : 
    QGraphicsView(parent)
{
    init();
}

void GraphicsView::init() {
    wheelPosition = 40.0;
    scaleFactor = 1.0;
    currentImage = new QGraphicsPixmapItem();
    prevImageWidth = 0;
    prevImageHeight = 0;
    helpTextItem = 0;
}

void GraphicsView::changeImage(QImage image) {
    scene()->clear(); //deletes the content of currentImage
    currentImage = scene()->addPixmap(QPixmap::fromImage(image));
    currentImage->setTransformationMode(Qt::SmoothTransformation);
    
    //when switching between zoomed-in images of the same size, the
    //zoom should not reset. Also, if the image is smaller than the 
    //graphicsscene it should not get "blown up" but stay at 1:1 size.
    if(image.width() != prevImageWidth || image.height() != prevImageHeight) {
        autoFit();
    }
    
    prevImageWidth = image.width();
    prevImageHeight = image.height();
}

void GraphicsView::autoFit() {
    QPixmap image = currentImage->pixmap();
    
    if(image.width() < this->width() && image.height() < this->height()) {
        resetImageScale();
    }
    else {
        fitImageInView();
    }
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
        break;
    case Qt::Key_C:
        //test if control is pressed as well
        if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
            emit controlCPressed();
        }
        break;
    }
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event) {
    emit doubleClicked();
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
    zoom(event->delta());
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {
    dragStart = event->pos();
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event) {
    int dragDistance = (dragStart - event->pos()).manhattanLength();
    
    if(dragDistance >= QApplication::startDragDistance())
        emit dragToFolderEvent();
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        //rightclick -> reset image scale to 1:1
        resetImageScale();
    }
    else if(event->button() == Qt::MiddleButton) {
        //middle click -> fit image to view
        fitImageInView();
    }
}

void GraphicsView::zoom(int wheelAngle) {
    //wheelAngle aka delta > 0 means forward (zoom in), < 0 means backwards (zoom out)
    if(wheelAngle > 0)
        wheelPosition += 1.0;
    else
        wheelPosition -= 1.0;
    
    scaleFactor = calcScaleFactor(wheelPosition);
    
    resetTransform();
    scale(scaleFactor, scaleFactor);
    
    //turn off AA when zooming in beyond 100%
    if(scaleFactor > 1.0)
        currentImage->setTransformationMode(Qt::FastTransformation);
    else
        currentImage->setTransformationMode(Qt::SmoothTransformation);
    
    emit scaleChanged(scaleFactor);
}

void GraphicsView::resetImageScale() {
    //adapt scene's bounding rect to image
    scene()->setSceneRect(QRectF(0, 0, currentImage->pixmap().width(), currentImage->pixmap().height()));
    //fit scene into graphicsview
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    
    resetTransform();
    centerOn(currentImage);
    
    scaleFactor = 1.0;
    wheelPosition = 40.0; //the same as calcWheelPosition(1.0);
    
    emit scaleChanged(scaleFactor);
}

void GraphicsView::fitImageInView() {
    //remove scrollbars before calculating the needed scenerect
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    //adapt scene's bounding rect to image
    scene()->setSceneRect(QRectF(0, 0, currentImage->pixmap().width(), currentImage->pixmap().height()));
    //fit scene into graphicsview
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    
    //compute current scaleFactor and corresponding wheel position
    double width = (double)scene()->width() / (double)this->width();
    double height = (double)scene()->height() / (double)this->height();
    scaleFactor = (height > width) ? (1.0 / height) : (1.0 / width);
    wheelPosition = calcWheelPosition(scaleFactor);
    
    //re-enable scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    emit scaleChanged(scaleFactor);
}

double GraphicsView::calcScaleFactor(double wheelPos) {
    // f(x) = c * a^x   where a = 2^(1/8) and c = (0.25 / a^24)
    // simplified: f(x) = (0.25 / (2^(1/8))^(24)) * (2^(1/8))^x
    
    double a = pow(2.0, (1.0 / 8.0));
    double c = 0.25 / pow(a, 24.0);
    
    return c * pow(a, wheelPos);
}

double GraphicsView::calcWheelPosition(double scaleFac) {
    //calculates x to a given y for the function in calcScaleFactor()
    double a = pow(2.0, (1.0 / 8.0));
    double c = 0.25 / pow(a, 24.0);
    
    double log_a = log(a);
    double log_c = log(c);
    
    return (log(scaleFac) - log_c) / log_a;
}

double GraphicsView::getScaleFactor() {
    return scaleFactor;
}

//display readme text in graphicsview
void GraphicsView::showHelp() {
    if(!helpTextItem) {
        scene()->clear();
        
        //load readme
        QString readmeText("Start by dropping images here");
        
        QFile readme(QApplication::applicationDirPath().append("/readme.txt"));
        
        if (readme.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&readme);
            readmeText = in.readAll();
        }
        
        helpTextItem = scene()->addSimpleText(readmeText);
        helpTextItem->setBrush(QColor(255, 255, 255)); 
    }
}

void GraphicsView::showText(QString text, QColor color) {
    scene()->clear();
    
    QGraphicsSimpleTextItem *textItem = scene()->addSimpleText(text);
    textItem->setBrush(color);
}
