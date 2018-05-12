#include "extractwindow.h"
#include "ui_extractwindow.h"
#include "clientthread.h"
#include "paramdialog.h"
#include "gotodialog.h"
#include <QFileDialog>
#include <time.h>

extern ClientThread ct;
extern ObjectDB odb;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    status_label = new QLabel;
    status_label->setMinimumSize(200, 20);
    ui->statusBar->addWidget(status_label);
	monkey_test_id = 0;
    warning_idx = 0;
    setCentralWidget(&views);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionTrain_triggered()
{
    ParamDialog param_dlg;
    if (param_dlg.exec() == QDialog::Accepted) {
		ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
		Q_ASSERT(connect_view != NULL);
        if (param_dlg.choose_cell)
            connect_view->cell_train(0, 0, 0, 0, param_dlg.cell_param1,
                param_dlg.cell_param2, param_dlg.cell_param3);
    }
}

void MainWindow::on_actionExtract_triggered()
{
    ParamDialog param_dlg;
    if (param_dlg.exec() == QDialog::Accepted) {
		ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
		Q_ASSERT(connect_view != NULL);
        if (param_dlg.choose_cell)
            connect_view->cell_extract(0, 0, 0, 0, param_dlg.cell_param1,
                param_dlg.cell_param2, param_dlg.cell_param3);
		else {
			VWSearchRequest vwsr;
			param_dlg.ep.get_param(param_dlg.action_name, vwsr.lpa);
            connect_view->wire_extract(vwsr, param_dlg.parallel ? 1 : 0);
		}
    }

}

void MainWindow::on_actionMark_Cell_triggered()
{
	ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
	Q_ASSERT(connect_view != NULL);
    connect_view->set_mark(OBJ_AREA, AREA_LEARN);
}

void MainWindow::on_actionMark_Extract_Area_triggered()
{
	ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
	Q_ASSERT(connect_view != NULL);
    connect_view->set_mark(OBJ_AREA, AREA_EXTRACT);
}

void MainWindow::on_actionLoad_Objects_triggered()
{
	ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
	Q_ASSERT(connect_view != NULL);
    QString image_file_name = QFileDialog::getOpenFileName( this,
                            "open file",
                            "C:/chenyu/work/ChipPintu/ViaWireExtract/");
    connect_view->load_objects(image_file_name.toStdString());
    warning_objs.clear();
    warning_idx = 0;
}

void MainWindow::mouse_change(QPoint pos, QString msg)
{
    char s[200];
    sprintf(s, "x%3d:%d,y%3d:%d, %s", pos.x() / 1024, pos.x(), pos.y() /1024, pos.y(), msg.toStdString().c_str());
    status_label->setText(s);
}

void MainWindow::on_actionClear_Objects_triggered()
{
	ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
	Q_ASSERT(connect_view != NULL);
    warning_objs.clear();
    warning_idx = 0;
    connect_view->clear_objs();
}

void MainWindow::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == monkey_test_id) {
		int r = rand() % 1024;
		QKeyEvent * key = NULL;
		if (r < 512) {
			switch (r % 4) {
			case 0:
				key = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
				break;
			case 1:
				key = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
				break;
			case 2:
				key = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
				break;
			case 3:
				key = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
				break;
			}
		}
		else {
			if (r < 640) {
				switch (r % 7) {
				case 0:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier);
					break;
				case 1:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier);
					break;
				case 2:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_2, Qt::NoModifier);
					break;
				case 3:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_3, Qt::NoModifier);
					break;
				case 4:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_4, Qt::NoModifier);
					break;
				case 5:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_5, Qt::NoModifier);
					break;
				case 6:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_6, Qt::NoModifier);
					break;
				case 7:
					key = new QKeyEvent(QEvent::KeyPress, Qt::Key_7, Qt::NoModifier);
					break;
				}
			} else
				if (r < 704) {
					if (r % 2)
						key = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
					else
						key = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
                } else
                    if (r < 736)
                        key = new QKeyEvent(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier);
		}

		if (key!=NULL)
			QCoreApplication::postEvent(views.currentWidget(), key);
	}
}
void MainWindow::on_actionMonkeyTest_triggered()
{
	if (monkey_test_id == 0) {
		srand(time(0));
		monkey_test_id = startTimer(200);
	}
	else {
		killTimer(monkey_test_id);
		monkey_test_id = 0;
	}

}

void MainWindow::on_actionNew_View_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "./",
                                                    tr("Project (*.prj)"));

    ConnectView * connect_view = new ConnectView(fileName.toStdString().c_str(), this);

    connect(connect_view, SIGNAL(mouse_change(QPoint, QString)), this, SLOT(mouse_change(QPoint, QString)));
    views.addWidget(connect_view);
	views.setCurrentWidget(connect_view);
	connect_view->setFocus();
}

void MainWindow::on_actionClose_View_triggered()
{
    if (views.count() == 0)
        return;
    ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
	views.removeWidget(connect_view);
	delete connect_view;
}

void MainWindow::on_actionNext_View_triggered()
{
	if (views.count() == 1)
		return;
	int idx = (views.currentIndex() + 1) % views.count();
    views.setCurrentIndex(idx);
}

void MainWindow::on_actionGoTo_triggered()
{
    GoToDialog goto_dlg;
    if (goto_dlg.exec() == QDialog::Accepted) {
        ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
        connect_view->goto_xy(goto_dlg.x, goto_dlg.y, -1);
    }
}

void MainWindow::on_actionNextWarning_triggered()
{
    if (warning_objs.empty()) {
		vector<ElementObj*> result;
        odb.get_objects(OBJ_AREA, AREA_CELL_MASK, 0.9f, warning_objs);
		odb.get_objects(OBJ_LINE, LINE_WIRE_AUTO_EXTRACT_MASK, 0.9f, result);
		warning_objs.insert(warning_objs.end(), result.begin(), result.end());
		odb.get_objects(OBJ_POINT, POINT_VIA_AUTO_EXTRACT_MASK, 0.9f, result);
		warning_objs.insert(warning_objs.end(), result.begin(), result.end());
    }
    if (warning_idx >= (int) warning_objs.size())
        warning_idx = 0;
    if (!warning_objs.empty()) {
        ConnectView * connect_view = qobject_cast<ConnectView *> (views.currentWidget());
        QPoint p0 = warning_objs[warning_idx]->p0;
        connect_view->goto_xy(p0.x(), p0.y(), warning_objs[warning_idx]->type3);
    }
    warning_idx++;

}
