#ifndef EXTRACTWINDOW_H
#define EXTRACTWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include "renderimage.h"
#include "connectview.h"
#include "searchobject.h"
#include "objectdb.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionTrain_triggered();
    void on_actionExtract_triggered();
    void mouse_change(QPoint pos, QString msg);

	void notify_progress(float progress);

    void on_actionLoad_Objects_triggered();

    void on_actionMark_Extract_Area_triggered();

    void on_actionMark_Cell_triggered();

    void on_actionClear_Objects_triggered();

    void on_actionMonkeyTest_triggered();

    void on_actionNew_View_triggered();

    void on_actionClose_View_triggered();

    void on_actionNext_View_triggered();

    void on_actionGoTo_triggered();

    void on_actionNextWarning_triggered();

    void on_actionClear_Wire_Via_triggered();

    void on_actionMark_Via_triggered();

    void on_actionMark_NoVia_triggered();

    void on_actionMark_Wire_Insu_Edge_triggered();

private:
    QLabel *status_label;
	QProgressBar * pProgressBar;
    Ui::MainWindow *ui;
    vector<ElementObj*> warning_objs;
    int warning_idx;

protected:
	void timerEvent(QTimerEvent *e);
	int monkey_test_id;

protected:
	QStackedWidget views;
};

#endif // EXTRACTWINDOW_H
