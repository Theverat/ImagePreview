#include "imagehandler.h"
#include "convertimagesdialog.h"
#include "cursormanager.h"
#include "exifparser.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QImageReader>
#include <QMovie>

#include <iostream>

ImageHandler::ImageHandler() {
    connect(&fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadModifiedImage(QString)));
}

ImageHandler::ImageHandler(GraphicsView *view, QWidget *parent){
    this->view = view;
    this->parent = parent;
    connect(&fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadModifiedImage(QString)));
}

void ImageHandler::setFileQueue(QList<QUrl> queue) {
    fileQueue = queue;
}

bool ImageHandler::load(QUrl url, bool suppressErrors){
    if(!url.isValid()) {
        throw "[load] invalid url!";
        return false;
    }

    QImageReader reader(url.toLocalFile());
    reader.setAllocationLimit(2000);

    image = reader.read();

    if(image.isNull() && !suppressErrors) {
        QMessageBox::information(parent, "Error while loading image",
                                 "Image not loaded!\nError: " + reader.errorString());
        return false;
    }

    if(reader.supportsAnimation()) {
        //animated gif
        QMovie *gif = new QMovie(url.toLocalFile());
        view->changeImage(gif, image);
    }
    else {
        //normal image
        //check exif data for image rotation

        ExifParser exifParser(url);
        if(exifParser.isValidExifData()) {
            //rotate image

            QTransform transform;

            switch(exifParser.getOrientation()) {
            case 1:
                break;
            case 2:
                image = image.mirrored(true, false);
                break;
            case 3:
                transform.rotate(180);
                break;
            case 4:
                image = image.mirrored(false, true);
                break;
            case 5:
                transform = transform.transposed();
                break;
            case 6:
                transform.rotate(90);
                break;
            case 7:
                transform.rotate(-90);
                image = image.mirrored(false, true);
                break;
            case 8:
                transform.rotate(270);
                break;
            }

            image = image.transformed(transform);
        }

        //display the image in the graphicsview
        view->changeImage(image);
    }

    //store the path the image was loaded from (for saving later)
    imageUrl = url;
    rotated = false;

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
    QList<QUrl> images = getImagesInDir(imageUrl.adjusted(QUrl::RemoveFilename));
    
    //if there are no images or just one, do nothing
    if(images.size() < 2)
        return;
    
    // Find out where we currently are in the list
    int current = images.indexOf(imageUrl);
    
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
        CursorManager::showCursor();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(parent, "Save Rotated Image",
                                      "The image was rotated. Do you want to save it?\n\
                                      WARNING: this will delete EXIF data!",
                                      QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if (reply == QMessageBox::Yes) {
            save(imageUrl.toLocalFile(), 98);
        }

        CursorManager::restoreCursorVisibility();
    }
    
    //remove current image from fileSystemWatcher
    fileSystemWatcher.removePath(imageUrl.toLocalFile());

    //construct the new Url and load the file
    //QUrl neighbourUrl = QUrl::fromLocalFile(imageUrl.adjusted(QUrl::RemoveFilename).toLocalFile() + images.at(current));
    load(images.at(current), true);
}

void ImageHandler::loadImage(QUrl url) {
    load(url);
}

void ImageHandler::reloadModifiedImage(QString path) {
    if(QImage(path).isNull())
        return;

    load(QUrl::fromLocalFile(path));
}

//returns a QStringList that contains all names of images in the folder
QList<QUrl> ImageHandler::getImagesInDir(QUrl url) {
    QStringList nameFilter;
    nameFilter << "*.png" << "*.jpg" << "*.jpeg" << "*.tiff" << "*.tif"
               << "*.ppm" << "*.bmp" << "*.xpm" << "*.psd" << "*.psb" << "*.gif";
    
    if(fileQueue.size() > 0) {
        return fileQueue;
    } else {
        // Cycle through all images in the directory
        QDir directory(url.toLocalFile());
        // The entryList only contains filenames, not full paths
        QStringList entryList = directory.entryList(nameFilter, QDir::Files);
        // Construct a list of full QUrls
        QList<QUrl> files;
        for(int i = 0; i < entryList.size(); ++i) {
            files.push_back(QUrl::fromLocalFile(url.toLocalFile() + entryList.at(i)));
        }
        return files;
    }
}

const QImage& ImageHandler::getImage() const {
    return image;
}

QUrl ImageHandler::getImageUrl() const {
    return imageUrl;
}

void ImageHandler::save(QString path, int quality) const {
    if(!image.save(path, 0, quality))
        QMessageBox::information(parent, "Error while saving Image", "Image not saved!");
}

void ImageHandler::save() {
    CursorManager::showCursor();

    QUrl url = QFileDialog::getSaveFileUrl(parent,
                                           "Save as",
                                           imageUrl,
                                           "Image Formats (*.png *.jpg *.jpeg *.tiff *.tif *.ppm *.bmp *.xpm)");
    
    //if saving process was aborted
    if(!url.isValid() || url.toLocalFile().isEmpty())
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

    CursorManager::restoreCursorVisibility();
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

        CursorManager::showCursor();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(parent, "Error",
                                      "Could not move image to trash!\nDo you want to delete it directly?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile file(fileToTrash.toLocalFile());
            file.remove();
        }

        CursorManager::restoreCursorVisibility();
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

void ImageHandler::toggleMarkCurrentImage() {
    if (markedFiles.contains(imageUrl)) {
        markedFiles.remove(imageUrl);
    } else if (imageUrl.isValid()) {
        markedFiles.insert(imageUrl);
    }
}
