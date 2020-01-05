#include "paramdialog.h"
#include "ui_paramdialog.h"
#include <QFileDialog>

ParamDialog::ParamDialog(vector<int> & dia_, int _layer_min, int _layer_max, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParamDialog)
{
    ui->setupUi(this);
	file_name = "./action.xml";
	update_action();
	memset(dia, 0, sizeof(dia));
	for (int i = 0; i < min(sizeof(dia) / sizeof(int), dia_.size()); i++)
		dia[i] = dia_[i];
	if (_layer_min < 0 && _layer_max < 0) {
		ui->dia0->setText(QString::number(dia[0]));
		ui->dia1->setText(QString::number(dia[1]));
		ui->dia2->setText(QString::number(dia[2]));
		ui->dia3->setText(QString::number(dia[3]));
		ui->dia4->setText(QString::number(dia[4]));
		ui->dia5->setText(QString::number(dia[5]));
		ui->dia6->setText(QString::number(dia[6]));
		ui->dia7->setText(QString::number(dia[7]));
		ui->dia8->setText(QString::number(dia[8]));
		ui->dia9->setText(QString::number(dia[9]));
		ui->dia10->setText(QString::number(dia[10]));
		ui->dia11->setText(QString::number(dia[11]));
		ui->dia12->setText(QString::number(dia[12]));
		ui->dia13->setText(QString::number(dia[13]));
		ui->dia14->setText(QString::number(dia[14]));
		ui->layer_min->setEnabled(false);
		ui->layer_max->setEnabled(false);
	}
	else {
		ui->dia0->setEnabled(false);
		ui->dia1->setEnabled(false);
		ui->dia2->setEnabled(false);
		ui->dia3->setEnabled(false);
		ui->dia4->setEnabled(false);
		ui->dia5->setEnabled(false);
		ui->dia6->setEnabled(false);
		ui->dia7->setEnabled(false);
		ui->dia8->setEnabled(false);
		ui->dia9->setEnabled(false);
		ui->dia10->setEnabled(false);
		ui->dia11->setEnabled(false);
		ui->dia12->setEnabled(false);
		ui->dia13->setEnabled(false);
		ui->dia14->setEnabled(false);
		ui->layer_min->setText(QString::number(_layer_min));
		ui->layer_max->setText(QString::number(_layer_max));
	}


    cell_param1 = 0.1;
    cell_param2 = 3;
    cell_param3 = 0;
    choose = 0;
    parallel = false;
	ui->file_name->setText(QString::fromStdString(file_name));
	ui->cell_param1->setText(QString::number(cell_param1));
    ui->cell_param2->setText(QString::number(cell_param2));
    ui->cell_param3->setText(QString::number(cell_param3));	
    ui->parallel->setChecked(parallel);
	ui->ml_parallel->setChecked(parallel);
}

ParamDialog::~ParamDialog()
{
    delete ui;
}

void ParamDialog::on_pushButton_clicked()
{
    choose = 0;
	action_name = ui->actions->currentItem()->text().toStdString();
    cell_param1 = ui->cell_param1->text().toDouble();
    cell_param2 = ui->cell_param2->text().toDouble();
    cell_param3 = ui->cell_param3->text().toDouble();
    parallel = ui->parallel->isChecked();
    accept();
}

void ParamDialog::on_pushButton_2_clicked()
{
    choose = 1;

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

void ParamDialog::on_pushButton_4_clicked()
{
	choose = 2;
    dia[0] = ui->dia0->text().toInt();
    dia[1] = ui->dia1->text().toInt();
    dia[2] = ui->dia2->text().toInt();
    dia[3] = ui->dia3->text().toInt();
    dia[4] = ui->dia4->text().toInt();
    dia[5] = ui->dia5->text().toInt();
    dia[6] = ui->dia6->text().toInt();
    dia[7] = ui->dia7->text().toInt();
    dia[8] = ui->dia8->text().toInt();
    dia[9] = ui->dia9->text().toInt();
    dia[10] = ui->dia10->text().toInt();
    dia[11] = ui->dia11->text().toInt();
    dia[12] = ui->dia12->text().toInt();
    dia[13] = ui->dia13->text().toInt();
    dia[14] = ui->dia14->text().toInt();
	layer_min = ui->layer_min->text().toInt();
	layer_max = ui->layer_max->text().toInt();
	parallel = ui->ml_parallel->isChecked();
    accept();
}
