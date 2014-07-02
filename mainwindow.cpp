#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsscene.h"
#include "convertimagesdialog.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSettings>
#include <QKeyEvent>
#include <QFileDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fittedInView(false),
    zoomed(false)
{
    ui->setupUi(this);
    
    //initialize graphicsview
    GraphicsScene *scene = new GraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::black));
    ui->graphicsView->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setAcceptDrops(true);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    
    //initialize imageHandler
    imageHandler = new ImageHandler(scene);
    
    //connect signals/slots
    //graphicsview drag and drop
    connect(ui->graphicsView, SIGNAL(singleImageDropped(QUrl)), imageHandler, SLOT(loadImage(QUrl)));
    //keyboard shortcuts
    connect(ui->graphicsView, SIGNAL(keyLeftPressed()), imageHandler, SLOT(previous()));
    connect(ui->graphicsView, SIGNAL(keyRightPressed()), imageHandler, SLOT(next()));
    connect(ui->graphicsView, SIGNAL(controlSPressed()), imageHandler, SLOT(save()));
    connect(ui->graphicsView, SIGNAL(controlCPressed()), this, SLOT(convertImages()));
    //zoom in/out, reset view, fit in view
    connect(ui->graphicsView, SIGNAL(mouseWheelZoom(bool)), this, SLOT(zoom(bool)));
    connect(ui->graphicsView, SIGNAL(rightClick()), this, SLOT(resetZoom()));
    connect(ui->graphicsView, SIGNAL(middleClick()), this, SLOT(fitInView()));
    //display image info, fit image into view etc.
    connect(imageHandler, SIGNAL(imageLoaded()), this, SLOT(initImageLoaded()));
    //open in file browser
    connect(ui->pushButton_openFolder, SIGNAL(clicked()), this, SLOT(openFolder()));
    
    //read last window position from registry
    readPositionSettings();
    
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
    
    //set initial zoom, fit image into window if it is too large
    if(!zoomed) {
        fitInView();
        if(image.width() < ui->graphicsView->width() && image.height() < ui->graphicsView->height()) {
            resetZoom();
            zoomed = false;
        }
    }
    
    displayImageInfo(image, imageUrl);
    
    //use loaded image as application icon
    QIcon icon(QPixmap::fromImage(image.scaled(64, 64, Qt::KeepAspectRatio)));
    this->setWindowIcon(icon);
    //use the name of the loaded image as window title
    this->setWindowTitle(QFileInfo(imageUrl.toLocalFile()).fileName() + " - Image Preview Tool");
}

//creates the info text for the label and displays it
void MainWindow::displayImageInfo(QImage image, QUrl imageUrl) {
    QFileInfo fileInfo(imageUrl.toLocalFile());
    qint64 sizeBytes = fileInfo.size();
    double sizeKilobytes = (double)sizeBytes / 1024.0;
    
    ui->label_path->setText(imageUrl.toLocalFile());
    ui->label_size->setText(QString::number(image.width()) + " x " + QString::number(image.height()));
    ui->label_fileSize->setText(QString::number(sizeKilobytes) + " kB");
    
    ui->label_path->adjustSize();
}

void MainWindow::zoom(bool forward) {
    if(forward) {
        zoomIn();
    }
    else {
        zoomOut();
    }
}

void MainWindow::zoomIn() {
    ui->graphicsView->scale(1.2, 1.2);
    fittedInView = false;
    zoomed = true;
}

void MainWindow::zoomOut() {
    ui->graphicsView->scale(0.8, 0.8);
    fittedInView = false;
    zoomed = true;
}

//resets zoom to 1:1
void MainWindow::resetZoom() {
    ui->graphicsView->resetTransform();
    fittedInView = false;
    zoomed = true;
}

//fits the preview into the graphicsView
void MainWindow::fitInView() {
    QImage image = imageHandler->getImage();
    ui->graphicsView->scene()->setSceneRect(QRectF(0, 0, image.width(), image.height()));
    ui->graphicsView->setSceneRect(ui->graphicsView->scene()->sceneRect());
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
    fittedInView = true;
    zoomed = false;
}

void MainWindow::openFolder() {
    QUrl folderUrl = imageHandler->getImageUrl().adjusted(QUrl::RemoveFilename);
    
    if(folderUrl.isValid())
        QDesktopServices::openUrl(folderUrl);
}

void MainWindow::convertImages() {
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                                                     "Select Images to Convert",
                                                     imageHandler->getImageUrl().adjusted(QUrl::RemoveFilename),
                                                     "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    
    if(urls.size() == 0)
        return;
    
    ConvertImagesDialog *dialog = new ConvertImagesDialog(this, imageHandler, urls);
    dialog->show();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if(fittedInView)
        fitInView();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writePositionSettings();
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
