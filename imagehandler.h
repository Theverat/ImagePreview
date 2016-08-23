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
    ImageHandler(GraphicsView *view, QWidget *parent);
    void setFileQueue(QList<QUrl> queue);
    bool load(QUrl url, bool suppressErrors = false);
    const QImage& getImage() const;
    QUrl getImageUrl() const;
    void save(QString path, int quality = -1) const;
    TrashHandler* getTrashHandler();

private:
    QWidget *parent;
    GraphicsView *view;
    QImage image;
    QUrl imageUrl;
    QList<QUrl> fileQueue;
    QFileSystemWatcher fileSystemWatcher;
    TrashHandler trashHandler;
    bool rotated;
    
    QList<QUrl> getImagesInDir(QUrl url);
    void loadNeighbourImage(bool rightNeighbour);

public slots:
    void loadImage(QUrl url);
    void reloadModifiedImage(QString path);
    void next();
    void previous();
    void save();
    void deleteCurrent();
    void rotateCurrent();
    
signals:
    void imageLoaded();
};

#endif // IMAGEHANDLER_H
