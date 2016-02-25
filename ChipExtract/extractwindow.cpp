#include "extractwindow.h"
#include "ui_extractwindow.h"
#include "clientthread.h"

extern ClientThread ct;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    render_thread = new QThread;
    render_image = new RenderImage;
    connect_view = new ConnectView(this);
    qRegisterMetaType<RenderType>("RenderType");
    connect(connect_view, SIGNAL(render_bkimg(const unsigned char, const QRect, const QSize, RenderType, const QObject *, bool)),
            render_image, SLOT(render_bkimg(const unsigned char, const QRect, const QSize, RenderType, const QObject *, bool)));
    connect(render_image, SIGNAL(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const QObject *)),
            connect_view, SLOT(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const QObject *)));
    connect(&ct, SIGNAL(bkimg_packet_arrive(void *)), render_image, SLOT(bkimg_packet_arrive(void *)));
    connect(&ct, SIGNAL(server_connected()), render_image, SLOT(server_connected()));
    connect(&ct, SIGNAL(server_disconnected()), render_image, SLOT(server_disconnected()));
    connect(&ct, SIGNAL(server_connected()), connect_view, SLOT(server_connected()));
    connect(&ct, SIGNAL(server_disconnected()), connect_view, SLOT(server_disconnected()));
    setCentralWidget(connect_view);
    render_image->moveToThread(render_thread);
    render_thread->start();
}

MainWindow::~MainWindow()
{
    render_image->deleteLater();
    render_thread->quit();
    delete render_thread;
    delete ui;
}
