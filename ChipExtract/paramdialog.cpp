#include "paramdialog.h"
#include "ui_paramdialog.h"
#include <QFileDialog>

ParamDialog::ParamDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParamDialog)
{
    ui->setupUi(this);
	file_name = "./action.xml";
	update_action();

    cell_param1 = 0.1;
    cell_param2 = 3;
    cell_param3 = 0;
    choose_cell = false;
    parallel = false;
	ui->file_name->setText(QString::fromStdString(file_name));
	ui->cell_param1->setText(QString::number(cell_param1));
    ui->cell_param2->setText(QString::number(cell_param2));
    ui->cell_param3->setText(QString::number(cell_param3));	
    ui->parallel->setChecked(parallel);
}

ParamDialog::~ParamDialog()
{

    delete ui;
}

void ParamDialog::on_pushButton_clicked()
{
    choose_cell = false;
	action_name = ui->actions->currentItem()->text().toStdString();
    cell_param1 = ui->cell_param1->text().toDouble();
    cell_param2 = ui->cell_param2->text().toDouble();
    cell_param3 = ui->cell_param3->text().toDouble();
    parallel = ui->parallel->isChecked();
    accept();
}

void ParamDialog::on_pushButton_2_clicked()
{
    choose_cell = true;

    cell_param1 = ui->cell_param1->text().toDouble();
    cell_param2 = ui->cell_param2->text().toDouble();
    cell_param3 = ui->cell_param3->text().toDouble();
    accept();
}

void ParamDialog::on_actions_itemClicked(QListWidgetItem *item)
{
#if EXTRACT_PARAM == 1
    QListWidget * param = ui->params;
    int count = param->count();
    for (int i = 0; i < count; i++)
        delete param->takeItem(0);
	vector<string> param_items;
	ep.get_param_sets(item->text().toStdString(), param_items);
	for (int i = 0; i < param_items.size(); i++)
		param->addItem(QString::fromStdString(param_items[i]));
#endif
}

void ParamDialog::on_lineEdit_editingFinished()
{	
	file_name = ui->file_name->text().toStdString();
	update_action();
}

void ParamDialog::update_action()
{
#if EXTRACT_PARAM == 1
	ep.clear();
	ep.read_file(file_name);
	QListWidget * action = ui->actions;
	int count = action->count();
	for (int i = 0; i < count; i++)
		delete action->takeItem(0);
	vector<string> actions;
	ep.get_param_set_list(actions);
	for (int i = 0; i < actions.size(); i++)
		ui->actions->addItem(QString::fromStdString(actions[i]));

	QListWidget * param = ui->params;
	count = param->count();
	for (int i = 0; i < count; i++)
        delete param->takeItem(0);
#endif
}
void ParamDialog::on_pushButton_3_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		"./",
		tr("Project (*.xml)"));
    file_name = fileName.toStdString();
    ui->file_name->setText(fileName);
	update_action();
}
