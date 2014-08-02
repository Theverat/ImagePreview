#ifndef RESTORETRASHDIALOG_H
#define RESTORETRASHDIALOG_H

#include <QDialog>
#include "trashhandler.h"

namespace Ui {
class RestoreTrashDialog;
}

class RestoreTrashDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RestoreTrashDialog(QWidget *parent = 0, TrashHandler *trashHandler = 0);
    ~RestoreTrashDialog();

private:
    Ui::RestoreTrashDialog *ui;
    TrashHandler *trashHandler;

private slots:
    void restoreFile();
    void openTrashFolder();
};

#endif // RESTORETRASHDIALOG_H
