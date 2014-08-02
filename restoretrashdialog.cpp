#include "restoretrashdialog.h"
#include "ui_restoretrashdialog.h"
#include <QDesktopServices>
#include <iostream>

RestoreTrashDialog::RestoreTrashDialog(QWidget *parent, TrashHandler *trashHandler) :
    QDialog(parent),
    ui(new Ui::RestoreTrashDialog)
{
    ui->setupUi(this);
    this->trashHandler = trashHandler;

    connect(ui->pushButton_restoreFile, SIGNAL(clicked()), this, SLOT(restoreFile()));
    connect(ui->pushButton_openTrashFolder, SIGNAL(clicked()), this, SLOT(openTrashFolder()));
    connect(ui->pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButton_deleteAll, SIGNAL(clicked()), this, SLOT(accept()));

    foreach(TrashedFile file, trashHandler->getFiles()) {
        //create icon
        QPixmap pixmap(file.getUrl().toLocalFile());
        QIcon icon(pixmap.scaledToHeight(128));

        QListWidgetItem *item = new QListWidgetItem(file.getUrl().fileName());
        item->setIcon(icon);

        ui->listWidget->addItem(item);
    }
}

RestoreTrashDialog::~RestoreTrashDialog()
{
    delete ui;
}

void RestoreTrashDialog::restoreFile() {
    if(!trashHandler->isEmpty()) {
        int index = ui->listWidget->selectionModel()->selectedIndexes().at(0).row();

        if(trashHandler->restore(index)) {
            delete ui->listWidget->item(index);
        }
    }
}

void RestoreTrashDialog::openTrashFolder() {
    QUrl folderUrl = trashHandler->getTrashUrl();

    if(folderUrl.isValid())
        QDesktopServices::openUrl(folderUrl);
}
