#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsscene.h"
#include "convertimagesdialog.h"
#include "restoretrashdialog.h"
#include "cursormanager.h"

#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QMimeData>
#include <QDrag>
#include <QDesktopWidget>
#include <QCloseEvent>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //initialize graphicsview
    GraphicsScene *scene = new GraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::black));
    ui->graphicsView->setAcceptDrops(true);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setStyleSheet( "QGraphicsView { border-style: none; }" );
    ui->graphicsView->showHelp();
    
    //initialize imageHandler
    imageHandler = new ImageHandler(ui->graphicsView, this);
    
    //connect signals/slots
    //graphicsview drag and drop
    connect(ui->graphicsView, SIGNAL(singleImageDropped(QUrl)), imageHandler, SLOT(loadImage(QUrl)));
    //keyboard shortcuts
    connect(ui->graphicsView, SIGNAL(keyLeftPressed()), imageHandler, SLOT(previous()));
    connect(ui->graphicsView, SIGNAL(keyRightPressed()), imageHandler, SLOT(next()));
    connect(ui->graphicsView, SIGNAL(controlSPressed()), imageHandler, SLOT(save()));
    connect(ui->graphicsView, SIGNAL(controlCPressed()), this, SLOT(convertImages()));
    connect(ui->graphicsView, SIGNAL(deletePressed()), imageHandler, SLOT(deleteCurrent()));
    connect(ui->graphicsView, SIGNAL(rotatePressed()), imageHandler, SLOT(rotateCurrent()));
    //doubleclick -> fullscreen
    connect(ui->graphicsView, SIGNAL(doubleClicked()), this, SLOT(toggleFullscreen()));
    //display image info, update scale factor display
    connect(imageHandler, SIGNAL(imageLoaded()), this, SLOT(initImageLoaded()));
    connect(ui->graphicsView, SIGNAL(scaleChanged(double)), this, SLOT(displayImageInfo()));
    //open in file browser
    connect(ui->pushButton_openFolder, SIGNAL(clicked()), this, SLOT(openFolder()));
    //drag image (copy to folder)
    connect(ui->pushButton_drag, SIGNAL(pressed()), this, SLOT(dragToFolder()));
    //set scale (double spinbox)
    connect(ui->doubleSpinBox_scale, SIGNAL(editingFinished()), this, SLOT(setZoom()));
    
    //read last window position from registry
    readPositionSettings();
    
    //should not start as fullscreen
    this->setWindowState(Qt::WindowNoState);
    CursorManager::showCursor();
    
    //if the program was opened via "open with" by the OS, extract the image path from the arguments
    QStringList args = QCoreApplication::arguments();
    if(args.size() > 1) {
        imageHandler->loadImage(QUrl::fromLocalFile(args.at(1)));
        
        if(!isFullScreen()) {
            //adapt the size of the window to the image if it is smaller than the screen
            int imageWidth = imageHandler->getImage().width();
            int imageHeight = imageHandler->getImage().height();
            
            int screenWidth = QApplication::desktop()->width();
            int screenHeight = QApplication::desktop()->height();
            
            if(imageWidth < screenWidth - 100 && imageHeight < screenHeight - 100
                    && imageWidth > 255 && imageHeight > 255) {
                this->resize(imageWidth + 50, imageHeight + 50);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete imageHandler;
}

void MainWindow::initImageLoaded() {
    QImage image = imageHandler->getImage();
    QUrl imageUrl = imageHandler->getImageUrl();
    
    if(image.isNull()) {
        ui->graphicsView->showText("Could Not Load Image\n" + imageUrl.toLocalFile());
        return;
    }
    
    displayImageInfo();
    
    //use loaded image as application icon
    QIcon icon(QPixmap::fromImage(image.scaled(64, 64, Qt::KeepAspectRatioByExpanding)));
    this->setWindowIcon(icon);
    //use the name of the loaded image as window title
    this->setWindowTitle(QFileInfo(imageUrl.toLocalFile()).fileName() + " - Image Preview Tool");
}

//creates the info text for the label and displays it
void MainWindow::displayImageInfo() {
    QImage image = imageHandler->getImage();
    QUrl imageUrl = imageHandler->getImageUrl();
    
    QFileInfo fileInfo(imageUrl.toLocalFile());
    qint64 sizeBytes = fileInfo.size();
    double sizeKilobytes = (double)sizeBytes / 1024.0;
    
    ui->label_path->setText(imageUrl.toLocalFile());
    ui->label_size->setText(QString::number(image.width()) + " x " + QString::number(image.height()));
    ui->label_fileSize->setText(QString::number(sizeKilobytes, 'f', 2) + " kB");
    
    ui->doubleSpinBox_scale->setValue(ui->graphicsView->getScaleFactor() * 100.0);
}

void MainWindow::openFolder() {
    QUrl folderUrl = imageHandler->getImageUrl().adjusted(QUrl::RemoveFilename);
    
    if(folderUrl.isValid())
        QDesktopServices::openUrl(folderUrl);
}

void MainWindow::dragToFolder() {
    if(imageHandler->getImage().isNull())
        return;
    
    QMimeData *mimeData = new QMimeData;
    
    //pass url to mimedata
    QList<QUrl> urls;
    urls.append(imageHandler->getImageUrl());
    mimeData->setUrls(urls);
    
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    
    drag->exec(Qt::CopyAction);
}

void MainWindow::convertImages() {
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                                                    "Select Images to Convert",
                                                    imageHandler->getImageUrl().adjusted(QUrl::RemoveFilename),
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm *.psd *.psb)");
    
    if(urls.size() == 0)
        return;
    
    ConvertImagesDialog *dialog = new ConvertImagesDialog(this, imageHandler, urls);
    dialog->show();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    ui->graphicsView->autoFit();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    CursorManager::showCursor();

    //trash handling
    if(!imageHandler->getTrashHandler()->isEmpty()) {
        RestoreTrashDialog *dialog = new RestoreTrashDialog(this, imageHandler->getTrashHandler());
        if(dialog->exec() == 0) {
            //"cancel" was pressed -> mainwindow should not close
            event->ignore();
            return;
        }
    }

    //exit fullscreen
    if(isFullScreen())
        toggleFullscreen();

    writePositionSettings();
}

void MainWindow::toggleFullscreen() {
    if(isFullScreen()) {
        ui->widget_infobar->show();
        this->setWindowState(Qt::WindowNoState);
        CursorManager::showCursor();
    }
    else
    {
        ui->widget_infobar->hide();
        this->setWindowState(Qt::WindowFullScreen);
        CursorManager::hideCursor();
    }
    
    ui->graphicsView->autoFit();
}

void MainWindow::setZoom() {
    ui->graphicsView->zoom(ui->doubleSpinBox_scale->value() / 100.0);
}

//write window size, position etc. to registry
void MainWindow::writePositionSettings()
{
    QSettings qsettings( "simon", "imagepreview" );
    
    qsettings.beginGroup( "mainwindow" );
    
    qsettings.setValue( "geometry", saveGeometry() );
    qsettings.setValue( "savestate", saveState() );
    qsettings.setValue( "maximized", isMaximized() );
    if ( !isMaximized() ) {
        qsettings.setValue( "pos", pos() );
        qsettings.setValue( "size", size() );
    }
    
    qsettings.endGroup();
}

//read window size, position etc. from registry
void MainWindow::readPositionSettings()
{
    QSettings qsettings( "simon", "imagepreview" );
    
    qsettings.beginGroup( "mainwindow" );
    
    restoreGeometry(qsettings.value( "geometry", saveGeometry() ).toByteArray());
    restoreState(qsettings.value( "savestate", saveState() ).toByteArray());
    move(qsettings.value( "pos", pos() ).toPoint());
    resize(qsettings.value( "size", size() ).toSize());
    if ( qsettings.value( "maximized", isMaximized() ).toBool() )
        showMaximized();
    
    qsettings.endGroup();
}
