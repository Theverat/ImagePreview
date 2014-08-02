#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QImage>
#include <QUrl>
#include <QObject>
#include <QFileSystemWatcher>
#include "graphicsview.h"
#include "trashhandler.h"

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
    TrashHandler *getTrashHandler();

private:
    GraphicsView *view;
    QImage image;
    QUrl imageUrl;
    QFileSystemWatcher fileSystemWatcher;
    TrashHandler trashHandler;
    
    QStringList getImagesInDir(QUrl url);
    void loadNeighbourImage(bool rightNeighbour);

public slots:
    void loadImage(QUrl url);
    void reloadModifiedImage(QString path);
    void next();
    void previous();
    void save();
    void convertMultiple();
    void deleteCurrent();
    
signals:
    void imageLoaded();
};

#endif // IMAGEHANDLER_H
