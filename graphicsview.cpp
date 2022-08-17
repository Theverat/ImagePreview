#include "graphicsview.h"

#include <QFile>
#include <QMimeData>
#include <QDropEvent>
#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QTextStream>
#include <QLabel>
#include <QMovie>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QDebug>

#include <math.h>
#include <iostream>

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

void GraphicsView::changeImage(const QImage& image) {
    scene()->clear();
    currentImage = scene()->addPixmap(QPixmap::fromImage(image));

    //when switching between zoomed-in images of the same size, the
    //zoom should not reset. Also, if the image is smaller than the
    //graphicsscene it should not get "blown up" but stay at 1:1 size.
    if(image.width() != prevImageWidth || image.height() != prevImageHeight) {
        autoFit();
    }
    
    prevImageWidth = image.width();
    prevImageHeight = image.height();
}

void GraphicsView::changeImage(QMovie *gif, const QImage& firstFrame) {
    scene()->clear();
    currentImage = scene()->addPixmap(QPixmap::fromImage(firstFrame));
    currentImage->hide();

    QLabel *gif_anim = new QLabel();
    gif_anim->setMovie(gif);
    gif->start();
    scene()->addWidget(gif_anim);

    autoFit();

    prevImageWidth = 0;
    prevImageHeight = 0;
}

void GraphicsView::autoFit() {
    int width = currentImage->pixmap().width();
    int height = currentImage->pixmap().height();
    
    if(width < this->width() && height < this->height()) {
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
        if (urls.size() > 1) {
            emit multipleImagesDropped(urls);
        } else {
            emit singleImageDropped(urls.at(0));
        }
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
    case Qt::Key_R:
        emit rotatePressed();
        break;
    case Qt::Key_F:
        fitImageInView();
        break;
    case Qt::Key_Escape:
        emit doubleClicked();
        break;
    case Qt::Key_F11:
        emit doubleClicked();
        break;
    case Qt::Key_Delete:
        emit deletePressed();
        break;
    case Qt::Key_P:
        //test if control is pressed as well
        if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
            //show print dialog
            QPrinter printer(QPrinter::HighResolution);
            
            QPrintPreviewDialog preview(&printer);
            connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printPreview(QPrinter*)));
            preview.exec();
        }
    }
}

void GraphicsView::printPreview(QPrinter *printer) {
    //render the graphicsscene to show in print preview dialog
    QPainter painter(printer);
    this->scene()->render(&painter);
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event) {
    emit doubleClicked();
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
    const QPoint numPixels = event->pixelDelta();
    const QPoint numDegrees = event->angleDelta() / 8;
    int delta = 0;
    double speed = 1.0;
    
    if (!numPixels.isNull()) {
        delta = numPixels.y();
        speed = std::min(1.0, std::abs(delta) * 0.01);
    } else if (!numDegrees.isNull()) {
        delta = numDegrees.y();
    }
    
    if (delta > 0)
        wheelPosition += speed;
    else if (delta < 0)
        wheelPosition -= speed;
    
    scaleFactor = calcScaleFactor(wheelPosition);
    setScale();

    event->accept();
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        //rightclick -> reset image scale to 1:1
        resetImageScale();
        
        double centerX = (double)(currentImage->pixmap().width() / width()) * event->pos().x();
        double centerY = (double)(currentImage->pixmap().height() / height()) * event->pos().y();
        centerOn(centerX, centerY);
    }
    else if(event->button() == Qt::MiddleButton) {
        //middle click -> fit image to view
        fitImageInView();
    }
}

//turn off AA when zooming in beyond 100%
void GraphicsView::choosePixmapTransform() {
    if(scaleFactor < 2.0)
        currentImage->setTransformationMode(Qt::SmoothTransformation);
    else
        currentImage->setTransformationMode(Qt::FastTransformation);
}

void GraphicsView::zoom(int wheelAngle) {
    //wheelAngle aka delta > 0 means forward (zoom in), < 0 means backwards (zoom out)
    if(wheelAngle > 0)
        wheelPosition += 1.0;
    else
        wheelPosition -= 1.0;
    
    scaleFactor = calcScaleFactor(wheelPosition);
    
    setScale();
}

void GraphicsView::zoom(double scale) {
    if(scale == scaleFactor)
        return;
    
    scaleFactor = scale;
    wheelPosition = calcWheelPosition(scaleFactor);
    
    setScale();
}

void GraphicsView::setScale() {
    resetTransform();
    scale(scaleFactor, scaleFactor);
    
    emit scaleChanged(scaleFactor);
    choosePixmapTransform();
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
    //deprecated because of hardcoded 2 pixel border
    //  fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);

    //fit scene into graphicsview - workaround from
    //http://www.qtforum.org/article/35467/the-2px-border-problem-in-qgraphicsview.html
    QTransform matrix(1,0,0,0,1,0,0,0,1);
    qreal xscale = this->width() / this->sceneRect().width();
    qreal yscale = this->height() / this->sceneRect().height();
    xscale = yscale = qMin(xscale,yscale);
    matrix.scale(xscale,yscale);
    this->setTransform(matrix);

    //compute current scaleFactor and corresponding wheel position
    double width = (double)scene()->width() / (double)this->width();
    double height = (double)scene()->height() / (double)this->height();
    scaleFactor = (height > width) ? (1.0 / height) : (1.0 / width);
    wheelPosition = calcWheelPosition(scaleFactor);
    
    //re-enable scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    emit scaleChanged(scaleFactor);
    choosePixmapTransform();
}

double GraphicsView::calcScaleFactor(double wheelPos) const {
    // f(x) = c * a^x   where a = 2^(1/8) and c = (0.25 / a^24)
    // simplified: f(x) = (0.25 / (2^(1/8))^(24)) * (2^(1/8))^x
    
    const double a = pow(2.0, (1.0 / 8.0));
    const double c = 0.25 / pow(a, 24.0);
    
    return c * pow(a, wheelPos);
}

double GraphicsView::calcWheelPosition(double scaleFac) const {
    //calculates x to a given y for the function in calcScaleFactor()
    const double a = pow(2.0, (1.0 / 8.0));
    const double c = 0.25 / pow(a, 24.0);
    
    const double log_a = log(a);
    const double log_c = log(c);
    
    return (log(scaleFac) - log_c) / log_a;
}

double GraphicsView::getScaleFactor() const {
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
    
    resetImageScale();
}
