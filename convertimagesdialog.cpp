#include "convertimagesdialog.h"
#include "ui_convertimagesdialog.h"
#include <QFileInfo>
#include <iostream>

ConvertImagesDialog::ConvertImagesDialog(QWidget *parent, ImageHandler *imageHandler, QList<QUrl> urls) :
    QDialog(parent),
    ui(new Ui::ConvertImagesDialog),
    imageHandler(imageHandler),
    images(urls)
{
    ui->setupUi(this);
    
    connect(ui->pushButton_convert, SIGNAL(clicked()), this, SLOT(convert()));
    connect(ui->comboBox_format, SIGNAL(currentIndexChanged(QString)), this, SLOT(setQualityOptions(QString)));
}

ConvertImagesDialog::~ConvertImagesDialog()
{
    delete ui;
}

void ConvertImagesDialog::convert() {
    ui->progressBar->setValue(0);
    double percentageBase = 100.0 / images.size();
    
    for(int i = 0; i < images.size(); i++) {
        ui->progressBar->setValue((int)(percentageBase) * (i + 1));
        
        QUrl url = images.at(i);
        
        QFileInfo fileInfo(url.toLocalFile());
        
        QString newFilePath = fileInfo.path();
        QString newFileName = fileInfo.baseName() + ui->lineEdit_nameSuffix->text();
        QString newFileSuffix = ui->comboBox_format->currentText();
        
        QString savePath = newFilePath + "/" + newFileName + newFileSuffix;
        
        std::cout << "Saving File " << savePath.toStdString() << std::endl;
        imageHandler->save(savePath, ui->spinBox_jpgQuality->value());
    }
    
    ui->progressBar->setValue(100);
}

void ConvertImagesDialog::setQualityOptions(QString format) {
    bool isJpg = (format == ".jpg");
    
    ui->label_jpgQuality->setEnabled(isJpg);
    ui->spinBox_jpgQuality->setEnabled(isJpg);
}
