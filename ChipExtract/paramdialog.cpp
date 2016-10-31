#include "paramdialog.h"
#include "ui_paramdialog.h"

ParamDialog::ParamDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParamDialog)
{
    ui->setupUi(this);
    wire_width = 10;
    grid_width = 15;
    insu_width = 5;
    via_radius = 9;
    vw_param1 = 0.1;
    vw_param2 = 0;
    vw_param3 = 0.2;

    cell_param1 = 0.1;
    cell_param2 = 3;
    cell_param3 = 0;
    choose_cell = false;

    ui->wire_width->setText(QString::number(wire_width));
    ui->grid_width->setText(QString::number(grid_width));
    ui->insu_width->setText(QString::number(insu_width));
    ui->via_radius->setText(QString::number(via_radius));
    ui->vw_param1->setText(QString::number(vw_param1));
    ui->vw_param2->setText(QString::number(vw_param2));
    ui->vw_param3->setText(QString::number(vw_param3));
    ui->cell_param1->setText(QString::number(cell_param1));
    ui->cell_param2->setText(QString::number(cell_param2));
    ui->cell_param3->setText(QString::number(cell_param3));
}

ParamDialog::~ParamDialog()
{

    delete ui;
}

void ParamDialog::on_pushButton_clicked()
{
    choose_cell = false;

    wire_width = ui->wire_width->text().toInt();
    grid_width = ui->grid_width->text().toInt();
    insu_width = ui->insu_width->text().toInt();
    via_radius = ui->via_radius->text().toInt();
    vw_param1 = ui->vw_param1->text().toDouble();
    vw_param2 = ui->vw_param2->text().toDouble();
    vw_param3 = ui->vw_param3->text().toDouble();
    cell_param1 = ui->cell_param1->text().toDouble();
    cell_param2 = ui->cell_param2->text().toDouble();
    cell_param3 = ui->cell_param3->text().toDouble();
    accept();
}

void ParamDialog::on_pushButton_2_clicked()
{
    choose_cell = true;

    wire_width = ui->wire_width->text().toInt();
    grid_width = ui->grid_width->text().toInt();
    insu_width = ui->insu_width->text().toInt();
    via_radius = ui->via_radius->text().toInt();
    vw_param1 = ui->vw_param1->text().toDouble();
    vw_param2 = ui->vw_param2->text().toDouble();
    vw_param3 = ui->vw_param3->text().toDouble();
    cell_param1 = ui->cell_param1->text().toDouble();
    cell_param2 = ui->cell_param2->text().toDouble();
    cell_param3 = ui->cell_param3->text().toDouble();
    accept();
}
