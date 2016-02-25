#ifndef EXTRACTWINDOW_H
#define EXTRACTWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "renderimage.h"
#include "connectview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QThread * render_thread;
    RenderImage * render_image;
    ConnectView * connect_view;
    Ui::MainWindow *ui;
};

#endif // EXTRACTWINDOW_H
