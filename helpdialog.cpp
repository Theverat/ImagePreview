#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
    connect(ui->pushButton_close, SIGNAL(clicked()), this, SLOT(close()));
}

HelpDialog::~HelpDialog()
{
    delete ui;
}
