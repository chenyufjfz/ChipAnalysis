#include "extractwindow.h"
#include "ui_extractwindow.h"
#include "clientthread.h"

extern ClientThread ct;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<QSharedPointer<SearchResults>>();
    qRegisterMetaType<QSharedPointer<SearchRects>>();

    render_thread = new QThread;
    render_image = new RenderImage;
    connect_view = new ConnectView(this);
    search_object = new SearchObject;
    qRegisterMetaType<RenderType>("RenderType");

    connect(connect_view, SIGNAL(render_bkimg(const unsigned char, const QRect, const QSize, RenderType, const QObject *, bool)),
            render_image, SLOT(render_bkimg(const unsigned char, const QRect, const QSize, RenderType, const QObject *, bool)));
    connect(render_image, SIGNAL(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const QObject *)),
            connect_view, SLOT(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const QObject *)));
    connect(connect_view, SIGNAL(train_cell(unsigned char,unsigned char,unsigned char,unsigned char,unsigned char,QRect,float,float,float)),
            search_object, SLOT(train_cell(unsigned char,unsigned char,unsigned char,unsigned char,unsigned char,QRect,float,float,float)));
    connect(search_object, SIGNAL(extract_cell_done(QSharedPointer<SearchResults>)),
            connect_view, SLOT(extract_cell_done(QSharedPointer<SearchResults>)));
    connect(connect_view, SIGNAL(extract_cell(unsigned char,unsigned char,unsigned char,unsigned char,QSharedPointer<SearchRects>,float,float,float)),
            search_object, SLOT(extract_cell(unsigned char,unsigned char,unsigned char,unsigned char,QSharedPointer<SearchRects>,float,float,float)));

    connect(&ct, SIGNAL(bkimg_packet_arrive(void *)), render_image, SLOT(bkimg_packet_arrive(void *)));
    connect(&ct, SIGNAL(search_packet_arrive(void*)), search_object, SLOT(search_packet_arrive(void*)));
    connect(&ct, SIGNAL(server_connected()), render_image, SLOT(server_connected()));
    connect(&ct, SIGNAL(server_disconnected()), render_image, SLOT(server_disconnected()));
    connect(&ct, SIGNAL(server_connected()), connect_view, SLOT(server_connected()));
    connect(&ct, SIGNAL(server_disconnected()), connect_view, SLOT(server_disconnected()));
    connect(&ct, SIGNAL(server_connected()), search_object, SLOT(server_connected()));
    connect(&ct, SIGNAL(server_disconnected()), search_object, SLOT(server_disconnected()));

    setCentralWidget(connect_view);
    connect_view->setFocus();
    render_image->moveToThread(render_thread);
    search_object->moveToThread(render_thread);
    render_thread->start();
}

MainWindow::~MainWindow()
{
    search_object->deleteLater();
    render_image->deleteLater();
    render_thread->quit();
    delete render_thread;
    delete ui;
}

void MainWindow::on_actionTrain_triggered()
{
    if (param_dlg.exec() == QDialog::Accepted) {
        if (param_dlg.choose_cell)
            connect_view->train(true, 0, 0, 0, 0, param_dlg.cell_param1,
                param_dlg.cell_param2, param_dlg.cell_param3);
    }
}

void MainWindow::on_actionExtract_triggered()
{
    if (param_dlg.exec() == QDialog::Accepted) {
        if (param_dlg.choose_cell)
            connect_view->extract(true, 0, 0, 0, 0, param_dlg.cell_param1,
                param_dlg.cell_param2, param_dlg.cell_param3);
    }

}
