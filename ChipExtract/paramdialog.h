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
	ExtractParam ep;
	string action_name;
	string file_name;
    float cell_param1;
    float cell_param2;
    float cell_param3;
    bool choose_cell;

public:
    explicit ParamDialog(QWidget *parent = 0);
    ~ParamDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_actions_itemClicked(QListWidgetItem *item);

    void on_lineEdit_editingFinished();

    void on_pushButton_3_clicked();

private:
    Ui::ParamDialog *ui;
	void update_action();
};

#endif // PARAMDIALOG_H
