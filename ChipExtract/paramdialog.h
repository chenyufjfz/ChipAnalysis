#ifndef PARAMDIALOG_H
#define PARAMDIALOG_H

#include <QDialog>

namespace Ui {
class ParamDialog;
}

class ParamDialog : public QDialog
{
    Q_OBJECT
public:
    int wire_width;
    int grid_width;
    int insu_width;
    int via_radius;
    float vw_param1;
    float vw_param2;
    float vw_param3;
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

private:
    Ui::ParamDialog *ui;
};

#endif // PARAMDIALOG_H
