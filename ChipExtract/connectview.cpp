#include "connectview.h"
#include <QPainter>
#include "globalconst.h"

extern GlobalConst gcst;
//following parameter is for view_rect move
const int step_para =3;
const double min_scale = 0.5;
const double max_scale = 16;

ConnectView::ConnectView(QWidget *parent) : QWidget(parent)
{    
    scale = 1;
    resize(600, 600);
    center = QPoint(0,0);
    bk_layer = 0;
    render_bk_layer = 0;
    connect_to_server = false;
    setAutoFillBackground(false);
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );
}

void ConnectView::paintEvent(QPaintEvent *)
{
    QPoint ce(size().width()*scale/2, size().height()*scale/2);
    QSize s(size().width()*scale, size().height()*scale);
    view_rect = QRect(center - gcst.pixel2bu(ce), gcst.pixel2bu(s));
    if (view_rect.width() > gcst.tot_width_bu())
        view_rect.adjust(view_rect.width()-gcst.tot_width_bu(), 0, 0, 0);
    if (view_rect.height() > gcst.tot_height_bu())
        view_rect.adjust(0, view_rect.height()-gcst.tot_height_bu(), 0, 0);
    if (view_rect.left() < gcst.left_bound())
        view_rect.moveLeft(gcst.left_bound());
    if (view_rect.right() > gcst.right_bound())
        view_rect.moveRight(gcst.right_bound());
    if (view_rect.top() < gcst.top_bound())
        view_rect.moveTop(gcst.top_bound());
    if (view_rect.bottom() > gcst.bottom_bound())
        view_rect.moveBottom(gcst.bottom_bound());
    Q_ASSERT(view_rect.left()>=gcst.left_bound() && view_rect.right()<=gcst.right_bound() &&
             view_rect.top()>=gcst.top_bound() && view_rect.bottom()<=gcst.bottom_bound());

    if (!render_rect.contains(view_rect) || render_bk_layer !=bk_layer) {
        qDebug("request image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.right(), view_rect.bottom(), size().width(), size().height());
        emit render_bkimg(bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
        return;
    }

    QPainter painter(this);

    QRect source(render_img.width() * (view_rect.left() - render_rect.left()) / render_rect.width(),
                 render_img.height()* (view_rect.top() - render_rect.top()) /render_rect.height(),
                 render_img.width() * view_rect.width() /render_rect.width(),
                 render_img.height()* view_rect.height() /render_rect.height());
    painter.drawImage(QRect(0,0, width(), height()), render_img, source);
    emit render_bkimg(bk_layer, view_rect, size(), NO_NEED_RETURN, this, true);
}

void ConnectView::keyPressEvent(QKeyEvent *e)
{
    int step;
    switch (e->key()) {
    case Qt::Key_Left:
        step = view_rect.width() * step_para /10;
        view_rect.moveLeft(max(gcst.left_bound(), view_rect.left() - step));
        center = view_rect.center();
        break;
    case Qt::Key_Up:
        step = view_rect.height() * step_para /10;
        view_rect.moveTop(max(gcst.top_bound(), view_rect.top() - step));
        center = view_rect.center();
        break;
    case Qt::Key_Right:
        step = view_rect.width() * step_para /10;
        view_rect.moveRight(min(gcst.right_bound(), view_rect.right() + step));
        center = view_rect.center();
        break;
    case Qt::Key_Down:
        step = view_rect.height() * step_para /10;
        view_rect.moveBottom(min(gcst.bottom_bound(), view_rect.bottom() + step));
        center = view_rect.center();
        break;
    case Qt::Key_PageUp:
        if (scale<max_scale)
            scale = scale *2;        
        break;
    case Qt::Key_PageDown:
        if (scale>min_scale)
            scale = scale /2;
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }

    qDebug("Key press, s=%5.2f, center=(%d,%d)", scale, center.x(), center.y());

    update();
}

void ConnectView::server_connected()
{
    connect_to_server = true;
    render_rect = QRect(0,0,0,0);
    update();
}

void ConnectView::server_disconnected()
{
    connect_to_server = false;
}

void ConnectView::render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                                    QImage image, bool finish, const QObject * view)
{
    if ((QObject*) this != view)
        return;

    if ( !rect.contains(view_rect) || layer!=bk_layer || screen !=size()) {
        qWarning("update view receive obsolete image");
        return;
    } else
        qDebug("render image l=%d,(%d,%d,%d,%d)", layer,
               rect.left(), rect.top(),rect.right(), rect.bottom());
    render_rect = rect;
    render_img = image;
    render_bk_layer = layer;

    update();
}
