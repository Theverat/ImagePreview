#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsscene.h"
#include "convertimagesdialog.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QMimeData>
#include <QDrag>

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
    ui->graphicsView->showHelp();
    
    //initialize imageHandler
    imageHandler = new ImageHandler(ui->graphicsView);
    
    //connect signals/slots
    //graphicsview drag and drop
    connect(ui->graphicsView, SIGNAL(singleImageDropped(QUrl)), imageHandler, SLOT(loadImage(QUrl)));
    connect(ui->graphicsView, SIGNAL(dragToFolderEvent()), this, SLOT(dragToFolder()));
    //keyboard shortcuts
    connect(ui->graphicsView, SIGNAL(keyLeftPressed()), imageHandler, SLOT(previous()));
    connect(ui->graphicsView, SIGNAL(keyRightPressed()), imageHandler, SLOT(next()));
    connect(ui->graphicsView, SIGNAL(controlSPressed()), imageHandler, SLOT(save()));
    connect(ui->graphicsView, SIGNAL(controlCPressed()), this, SLOT(convertImages()));
    //doubleclick -> fullscreen
    connect(ui->graphicsView, SIGNAL(doubleClicked()), this, SLOT(toggleFullscreen()));
    //display image info, update scale factor display 
    connect(imageHandler, SIGNAL(imageLoaded()), this, SLOT(initImageLoaded()));
    connect(ui->graphicsView, SIGNAL(scaleChanged(double)), this, SLOT(displayImageInfo()));
    //open in file browser
    connect(ui->pushButton_openFolder, SIGNAL(clicked()), this, SLOT(openFolder()));
    
    //read last window position from registry
    readPositionSettings();
    //should not start as fullscreen
    this->setWindowState(Qt::WindowNoState);
    
    //if the program was opened via "open with" by the OS, extract the image path from the arguments
    QStringList args = QCoreApplication::arguments();
    if(args.size() > 1) {
        imageHandler->loadImage(QUrl::fromLocalFile(args.at(1)));
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
    ui->label_scale->setText(QString::number(ui->graphicsView->getScaleFactor() * 100.0) + "%");
}

void MainWindow::openFolder() {
    QUrl folderUrl = imageHandler->getImageUrl().adjusted(QUrl::RemoveFilename);
    
    if(folderUrl.isValid())
        QDesktopServices::openUrl(folderUrl);
}

void MainWindow::dragToFolder() {
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
    writePositionSettings();
}

void MainWindow::toggleFullscreen() {
    if(isFullScreen()) {
        ui->widget_infobar->show();
        this->setWindowState(Qt::WindowNoState);
    } 
    else 
    {
        ui->widget_infobar->hide();
        this->setWindowState(Qt::WindowFullScreen);
    }
    
    ui->graphicsView->autoFit();
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
