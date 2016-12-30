#ifndef EXTRACTWINDOW_H
#define EXTRACTWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include "renderimage.h"
#include "connectview.h"
#include "paramdialog.h"
#include "searchobject.h"

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

    void on_actionLoad_Objects_triggered();

    void on_actionMark_Extract_Area_triggered();

    void on_actionMark_Cell_triggered();

    void on_actionClear_Objects_triggered();

private:
    ParamDialog param_dlg;
    QLabel *status_label;
    QThread * render_thread;
    RenderImage * render_image;
    SearchObject * search_object;
    ConnectView * connect_view;
    Ui::MainWindow *ui;
};

#endif // EXTRACTWINDOW_H
