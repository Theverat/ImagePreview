#ifndef CONVERTIMAGESDIALOG_H
#define CONVERTIMAGESDIALOG_H

#include <QDialog>
#include "imagehandler.h"

namespace Ui {
class ConvertImagesDialog;
}

class ConvertImagesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConvertImagesDialog(QWidget *parent = 0, ImageHandler *imageHandler = 0, QList<QUrl> urls = QList<QUrl>());
    ~ConvertImagesDialog();
    void addImages(QList<QUrl> urls);
    
private:
    Ui::ConvertImagesDialog *ui;
    QList<QUrl> images;
    ImageHandler *imageHandler;
    
private slots:
    void convert();
    void setQualityOptions(QString format);
    
};

#endif // CONVERTIMAGESDIALOG_H
