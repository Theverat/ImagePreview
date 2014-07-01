#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <QLabel>
#include "mainwindow.h"

class ImageInfo
{
public:
    ImageInfo();
    ImageInfo(QMainWindow *parent);
    void setImageInfo(QImage image, QUrl imageUrl);
    
private:
    QMainWindow *parent;
};

#endif // IMAGEINFO_H
