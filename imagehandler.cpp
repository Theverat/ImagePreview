#include "imagehandler.h"
#include <QMessageBox>
#include <QDirIterator>
#include <QImageReader>
#include <QFileInfo>
#include <QGraphicsTextItem>
#include <iostream>
#include <QFileDialog>
#include <QInputDialog>
#include <QPushButton>
#include "convertimagesdialog.h"

ImageHandler::ImageHandler() {
    
}

ImageHandler::ImageHandler(GraphicsView *view){
    this->view = view;
}

bool ImageHandler::load(QUrl url){
    if(!url.isValid()) {
        throw "[load] invalid url!";
        return false;
    }
    
    //load image
    image = QImage(url.toLocalFile());
    
    if(image.isNull()) {
        QMessageBox::information(0, "Error while loading image",
                                 "Image not loaded!\nMost likely the image format is not supported.");
        return false;
    }
    
    //store the path the image was loaded from (for saving later)
    imageUrl = url;
    
    //display the image in the graphicsview
    view->changeImage(image);
    
    //tell the mainwindow the image was loaded
    emit imageLoaded();
    
    return true;
}

//loads the next image
void ImageHandler::next(){
    loadNeighbourImage(true);
}

//loads the previous image
void ImageHandler::previous(){
    loadNeighbourImage(false);
}

void ImageHandler::loadNeighbourImage(bool rightNeighbour) {
    QStringList images = getImagesInDir(imageUrl.adjusted(QUrl::RemoveFilename));
    
    //if there are no images or just one, do nothing
    if(images.size() < 2)
        return;
    
    QFileInfo fileInfo(imageUrl.toLocalFile());
    int current = images.indexOf(QRegExp(QRegExp::escape(fileInfo.fileName())));
    int neighbour = current;
    
    if(rightNeighbour)
        neighbour += 1;
    else
        neighbour -= 1;
    
    //if at beginning, take last element, if at end, take first element
    if(neighbour < 0)
        neighbour = images.size() - 1;
    else if(neighbour > images.size() - 1)
        neighbour = 0;
    
    //construct the new Url and load the file
    QUrl neighbourUrl = QUrl::fromLocalFile(imageUrl.adjusted(QUrl::RemoveFilename).toLocalFile() + images.at(neighbour));
    load(neighbourUrl);
}

void ImageHandler::loadImage(QUrl url) {
    load(url);
}

//returns a QStringList that contains all names of images in url (e.g. "test.png", "test2.png")
QStringList ImageHandler::getImagesInDir(QUrl url) {
    QStringList nameFilter;
    nameFilter << "*.png" << "*.jpg" << "*.jpeg" << "*.tiff" << "*.ppm" << "*.bmp" << "*.xpm" << "*.psd" << "*.psb";
    
    QDir directory(url.toLocalFile());
    
    return directory.entryList(nameFilter, QDir::Files);
}

QImage ImageHandler::getImage() {
    return image;
}

QUrl ImageHandler::getImageUrl() {
    return imageUrl;
}

void ImageHandler::save(QString path, int quality) {
    if(!image.save(path, 0, quality))
        QMessageBox::information(0, "Error while saving Image", "Image not saved!");
}

void ImageHandler::save() {
    QUrl url = QFileDialog::getSaveFileUrl(0,
                                           "Save as",
                                           imageUrl,
                                           "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    
    //if saving process was aborted
    if(!url.isValid())
        return;
    
    QString path = url.toLocalFile();
    
    //if no file suffix was chosen, automatically use the images original format
    QFileInfo file(path);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        path += QFileInfo(imageUrl.toLocalFile()).suffix();
    //if jpeg was chosen as format, display a quality choosing dialog
    int quality = -1;
    if(file.suffix().toLower() == "jpg" || file.suffix().toLower() == "jpeg") {
        bool ok;
        quality = QInputDialog::getInt(0, "Quality", "JPG Quality:", 98, 1, 100, 1, &ok);
        if(!ok)
            quality = 98;
    }
    
    save(path, quality);
}

void ImageHandler::convertMultiple() {
    
}
