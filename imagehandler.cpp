#include "imagehandler.h"
#include "convertimagesdialog.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>

#include <iostream>

ImageHandler::ImageHandler() {
    connect(&fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadModifiedImage(QString)));
}

ImageHandler::ImageHandler(GraphicsView *view, QWidget *parent){
    this->view = view;
    this->parent = parent;
    connect(&fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadModifiedImage(QString)));
}

bool ImageHandler::load(QUrl url, bool suppressErrors){
    if(!url.isValid()) {
        throw "[load] invalid url!";
        return false;
    }
    
    //load image
    image = QImage(url.toLocalFile());
    
    if(image.isNull() && !suppressErrors) {
        QMessageBox::information(parent, "Error while loading image",
                                 "Image not loaded!\nMost likely the image format is not supported.");
        
        return false;
    }
    
    //store the path the image was loaded from (for saving later)
    imageUrl = url;
    rotated = false;
    
    //display the image in the graphicsview
    view->changeImage(image);

    //add the image file to the fileSystemWatcher
    fileSystemWatcher.addPath(url.toLocalFile());
    
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
    
    //convert rightNeighbour to an int (left = -1, right = 1)
    int relativeIndex = -1;
    if(rightNeighbour)
        relativeIndex = 1;
    current += relativeIndex;
    
    //if at beginning, take last element, if at end, take first element
    if(current < 0)
        current = images.size() - 1;
    else if(current > images.size() - 1)
        current = 0;

    //if image was rotated, ask if it should be saved
    if(rotated) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(parent, "Save Rotated Image", "The image was rotated. Do you want to save it?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            save(imageUrl.toLocalFile(), 98);
        }
    }
    
    //remove current image from fileSystemWatcher
    fileSystemWatcher.removePath(imageUrl.toLocalFile());

    //construct the new Url and load the file
    QUrl neighbourUrl = QUrl::fromLocalFile(imageUrl.adjusted(QUrl::RemoveFilename).toLocalFile() + images.at(current));
    load(neighbourUrl, true);
}

void ImageHandler::loadImage(QUrl url) {
    load(url);
}

void ImageHandler::reloadModifiedImage(QString path) {
    if(QImage(path).isNull())
        return;

    load(QUrl::fromLocalFile(path));
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
        QMessageBox::information(parent, "Error while saving Image", "Image not saved!");
}

void ImageHandler::save() {
    QUrl url = QFileDialog::getSaveFileUrl(parent,
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
        quality = QInputDialog::getInt(parent, "Quality", "JPG Quality:", 98, 1, 100, 1, &ok);
        if(!ok)
            quality = 98;
    }
    
    save(path, quality);
}

void ImageHandler::convertMultiple() {
    
}

void ImageHandler::deleteCurrent() {
    if(!imageUrl.isValid())
        return;

    QUrl fileToTrash = imageUrl;
    
    if(getImagesInDir(imageUrl.adjusted(QUrl::RemoveFilename)).size() > 1) {
        next();
    }
    else {
        image = QImage();
        imageUrl = QUrl();
        
        view->showText("No images in current folder.\nDrop image here to open it.");
    }

    if(!trashHandler.moveToTrash(fileToTrash)) {
        //could not move image to trash
        //ask user if file should be removed directly

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(parent, "Error", "Could not move image to trash!\nDo you want to delete it directly?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile file(fileToTrash.toLocalFile());
            file.remove();
        }
    }
}

TrashHandler* ImageHandler::getTrashHandler() {
    return &trashHandler;
}

void ImageHandler::rotateCurrent() {
    QTransform transform;
    transform.rotate(90);
    image = image.transformed(transform);

    view->changeImage(image);
    rotated = true;
}
