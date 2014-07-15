#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QImage>
#include <QUrl>
#include <QObject>
#include "graphicsview.h"

class ImageHandler : public QObject
{
    Q_OBJECT

public:
    ImageHandler();
    ImageHandler(GraphicsView *view);
    bool load(QUrl url, bool suppressErrors = false);
    QImage getImage();
    QUrl getImageUrl();
    void save(QString path, int quality = -1);

private:
    GraphicsView *view;
    QImage image;
    QUrl imageUrl;
    
    QStringList getImagesInDir(QUrl url);
    void loadNeighbourImage(bool rightNeighbour);

public slots:
    void loadImage(QUrl url);
    void next();
    void previous();
    void save();
    void convertMultiple();
    void deleteCurrentOnDisk();
    
signals:
    void imageLoaded();
};

#endif // IMAGEHANDLER_H
