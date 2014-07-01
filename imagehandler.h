#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QImage>
#include <QUrl>
#include <QObject>
#include <QLabel>
#include "graphicsscene.h"

class ImageHandler : public QObject
{
    Q_OBJECT

public:
    ImageHandler();
    ImageHandler(GraphicsScene *scene);
    bool load(QUrl url);
    QImage getImage();
    QUrl getImageUrl();

private:
    GraphicsScene *scene;
    QImage image;
    QUrl imageUrl;
    
    QStringList getImagesInDir(QUrl url);
    void loadNeighbourImage(bool rightNeighbour);

public slots:
    void loadImage(QUrl url);
    void next();
    void previous();
    void saveImage();
    
signals:
    void imageLoaded();
};

#endif // IMAGEHANDLER_H
