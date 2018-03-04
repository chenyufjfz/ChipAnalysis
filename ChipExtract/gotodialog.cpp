#include "gotodialog.h"
#include "ui_gotodialog.h"

GoToDialog::GoToDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GoToDialog)
{
    ui->setupUi(this);
    x =0;
    y =0;

    ui->x_edit->setText(QString::number(x));
    ui->y_edit->setText(QString::number(y));
}

GoToDialog::~GoToDialog()
{
    delete ui;
}

void GoToDialog::on_buttonBox_accepted()
{
    x = ui->x_edit->text().toInt();
    y = ui->y_edit->text().toInt();
}
