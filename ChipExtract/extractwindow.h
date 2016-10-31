#ifndef EXTRACTWINDOW_H
#define EXTRACTWINDOW_H

#include <QMainWindow>
#include <QThread>
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

private:
    ParamDialog param_dlg;
    QThread * render_thread;
    RenderImage * render_image;
    SearchObject * search_object;
    ConnectView * connect_view;
    Ui::MainWindow *ui;
};

#endif // EXTRACTWINDOW_H
