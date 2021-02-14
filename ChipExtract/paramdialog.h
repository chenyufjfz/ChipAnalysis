#ifndef PARAMDIALOG_H
#define PARAMDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "extractparam.h"

namespace Ui {
class ParamDialog;
}

class ParamDialog : public QDialog
{
    Q_OBJECT
public:
#if EXTRACT_PARAM == 1
	ExtractParam ep;
#endif
	string action_name;
	string file_name;
    float cell_param1;
    float cell_param2;
    float cell_param3;
    int choose;
    bool parallel;
	int dia[15];
	int wide_x[10];
	int wide_y[10];
	int layer_min, layer_max;
public:
	explicit ParamDialog(vector<int> & dia_, vector<int> & wide_x_, vector<int> & wide_y_, int _layer_min, int _layer_max, QWidget *parent = 0);
    ~ParamDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_actions_itemClicked(QListWidgetItem *item);

    void on_lineEdit_editingFinished();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::ParamDialog *ui;
	void update_action();
};

#endif // PARAMDIALOG_H
