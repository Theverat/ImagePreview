#include "imageinfo.h"
#include <QUrl>
#include <QFileInfo>

ImageInfo::ImageInfo() {
    
}

ImageInfo::ImageInfo(QMainWindow *parent) {
    this->parent = parent;
}

//creates the info text for the label and displays it
void ImageInfo::setImageInfo(QImage image, QUrl imageUrl) {
    if(!label_imageInfo)
        return;
    
    QFileInfo fileInfo(imageUrl.toLocalFile());
    qint64 sizeBytes = fileInfo.size();
    qint64 sizeMegabytes = sizeBytes / 1024;
    
    QString infoText;
    infoText.append(imageUrl.toLocalFile());
    infoText.append(" ");
    infoText.append(QString::number(image.width()));
    infoText.append(" x ");
    infoText.append(QString::number(image.height()));
    infoText.append("(" + QString::number(sizeMegabytes));
    infoText.append(" MB)");
    
    label_imageInfo->setText(infoText);
}
