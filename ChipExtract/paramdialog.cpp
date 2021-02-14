#include "paramdialog.h"
#include "ui_paramdialog.h"
#include <QFileDialog>

ParamDialog::ParamDialog(vector<int> & dia_, vector<int> & wide_x_, vector<int> & wide_y_, int _layer_min, int _layer_max, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParamDialog)
{
    ui->setupUi(this);
	file_name = "./action.xml";
	update_action();
	memset(dia, 0, sizeof(dia));
	memset(wide_x, 5, sizeof(wide_x));
	memset(wide_y, 5, sizeof(wide_y));
	for (int i = 0; i < min(sizeof(dia) / sizeof(int), dia_.size()); i++)
		dia[i] = dia_[i];
	for (int i = 0; i < min(sizeof(wide_x) / sizeof(int), wide_x_.size()); i++)
		wide_x[i] = wide_x_[i];
	for (int i = 0; i < min(sizeof(wide_y) / sizeof(int), wide_y_.size()); i++)
		wide_y[i] = wide_y_[i];
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

		ui->wide_x0->setEnabled(false);
		ui->wide_x1->setEnabled(false);
		ui->wide_x2->setEnabled(false);
		ui->wide_x3->setEnabled(false);
		ui->wide_x4->setEnabled(false);
		ui->wide_x5->setEnabled(false);
		ui->wide_x6->setEnabled(false);
		ui->wide_x7->setEnabled(false);
		ui->wide_x8->setEnabled(false);
		ui->wide_x9->setEnabled(false);
		ui->wide_y0->setEnabled(false);
		ui->wide_y1->setEnabled(false);
		ui->wide_y2->setEnabled(false);
		ui->wide_y3->setEnabled(false);
		ui->wide_y4->setEnabled(false);
		ui->wide_y5->setEnabled(false);
		ui->wide_y6->setEnabled(false);
		ui->wide_y7->setEnabled(false);
		ui->wide_y8->setEnabled(false);
		ui->wide_y9->setEnabled(false);
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

		ui->wide_x0->setText(QString::number(wide_x[0]));
		ui->wide_x1->setText(QString::number(wide_x[1]));
		ui->wide_x2->setText(QString::number(wide_x[2]));
		ui->wide_x3->setText(QString::number(wide_x[3]));
		ui->wide_x4->setText(QString::number(wide_x[4]));
		ui->wide_x5->setText(QString::number(wide_x[5]));
		ui->wide_x6->setText(QString::number(wide_x[6]));
		ui->wide_x7->setText(QString::number(wide_x[7]));
		ui->wide_x8->setText(QString::number(wide_x[8]));
		ui->wide_x9->setText(QString::number(wide_x[9]));
		ui->wide_y0->setText(QString::number(wide_y[0]));
		ui->wide_y1->setText(QString::number(wide_y[1]));
		ui->wide_y2->setText(QString::number(wide_y[2]));
		ui->wide_y3->setText(QString::number(wide_y[3]));
		ui->wide_y4->setText(QString::number(wide_y[4]));
		ui->wide_y5->setText(QString::number(wide_y[5]));
		ui->wide_y6->setText(QString::number(wide_y[6]));
		ui->wide_y7->setText(QString::number(wide_y[7]));
		ui->wide_y8->setText(QString::number(wide_y[8]));
		ui->wide_y9->setText(QString::number(wide_y[9]));
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
	wide_x[0] = ui->wide_x0->text().toInt();
	wide_x[1] = ui->wide_x1->text().toInt();
	wide_x[2] = ui->wide_x2->text().toInt();
	wide_x[3] = ui->wide_x3->text().toInt();
	wide_x[4] = ui->wide_x4->text().toInt();
	wide_x[5] = ui->wide_x5->text().toInt();
	wide_x[6] = ui->wide_x6->text().toInt();
	wide_x[7] = ui->wide_x7->text().toInt();
	wide_x[8] = ui->wide_x8->text().toInt();
	wide_x[9] = ui->wide_x9->text().toInt();
	wide_y[0] = ui->wide_y0->text().toInt();
	wide_y[1] = ui->wide_y1->text().toInt();
	wide_y[2] = ui->wide_y2->text().toInt();
	wide_y[3] = ui->wide_y3->text().toInt();
	wide_y[4] = ui->wide_y4->text().toInt();
	wide_y[5] = ui->wide_y5->text().toInt();
	wide_y[6] = ui->wide_y6->text().toInt();
	wide_y[7] = ui->wide_y7->text().toInt();
	wide_y[8] = ui->wide_y8->text().toInt();
	wide_y[9] = ui->wide_y9->text().toInt();
	layer_min = ui->layer_min->text().toInt();
	layer_max = ui->layer_max->text().toInt();
	parallel = ui->ml_parallel->isChecked();
    accept();
}
