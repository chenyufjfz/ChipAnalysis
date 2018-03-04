#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

namespace Ui {
class GoToDialog;
}

class GoToDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToDialog(QWidget *parent = 0);
    ~GoToDialog();
    int x,y;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::GoToDialog *ui;
};

#endif // GOTODIALOG_H
