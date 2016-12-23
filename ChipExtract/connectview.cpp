#include "connectview.h"
#include "objectdb.h"
#include <QPainter>
#include "globalconst.h"
#include <QMessageBox>

extern GlobalConst gcst;
extern ObjectDB odb;
//following parameter is for view_rect move
const int step_para =3;
const double min_scale = 0.5;
const double max_scale = 16;

ConnectView::ConnectView(QWidget *parent) : QWidget(parent)
{    
    scale = 1;
    resize(600, 600);
    center = QPoint(1450000,600000);
    bk_layer = 0;
    render_bk_layer = 0;
    connect_to_server = false;
    setAutoFillBackground(false);
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );
    setMouseTracking(true);
}

void ConnectView::draw_element(QPainter &painter)
{
	vector<ElementObj*> rst;
	odb.get_objects(OBJ_AREA, AREA_CELL_MASK | AREA_LEARN_MASK | AREA_EXTRACT_MASK, view_rect, rst);
	painter.setBrush(QBrush(Qt::NoBrush));
	for (unsigned i = 0; i < rst.size(); i++) {
		switch (rst[i]->type2) {
		case AREA_CELL:
			painter.setPen(QPen(Qt::red, 1));
			break;
		case AREA_LEARN:
			painter.setPen(QPen(Qt::blue, 1));
			break;
		case AREA_EXTRACT:
			painter.setPen(QPen(Qt::yellow, 1));
			break;
		}
		QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
			height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
		QPoint p1(width() * (rst[i]->p1.x() - view_rect.left()) / view_rect.width(),
			height()* (rst[i]->p1.y() - view_rect.top()) / view_rect.height());
		painter.drawRect(QRect(p0, p1));
	}

    odb.get_objects(OBJ_LINE, LINE_WIRE_AUTO_EXTRACT_MASK, view_rect, rst);
    painter.setPen(QPen(Qt::blue, 1));
    if (rst.size()!=0)
        qDebug("draw %d lines", rst.size());
    for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type3 + 2 == bk_layer) { //TODO fix it
            QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
            QPoint p1(width() * (rst[i]->p1.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p1.y() - view_rect.top()) / view_rect.height());
            painter.drawLine(p0, p1);
        }
    }

    odb.get_objects(OBJ_POINT, POINT_VIA_AUTO_EXTRACT_MASK, view_rect, rst);
    painter.setPen(QPen(Qt::green, 1, Qt::DotLine));
    painter.setBrush(QBrush(Qt::NoBrush));
    if (rst.size()!=0)
        qDebug("draw %d vias", rst.size());
    for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type3 + 2 == bk_layer) { //TODO fix it
            QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
            painter.drawEllipse(p0, 9, 9);
        }
    }
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

    if ((render_rect.width() > view_rect.width() * 3 && render_rect.width() > 0x10000) ||
        (render_rect.height() > view_rect.height() * 3 && render_rect.height() > 0x10000)) {
        qDebug("request clear image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.right(), view_rect.bottom(), size().width(), size().height());
        emit render_bkimg(bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
    }
    QPainter painter(this);

    QRect source(render_img.width() * (view_rect.left() - render_rect.left()) / render_rect.width(),
                 render_img.height()* (view_rect.top() - render_rect.top()) /render_rect.height(),
                 render_img.width() * view_rect.width() /render_rect.width(),
                 render_img.height()* view_rect.height() /render_rect.height());
    painter.drawImage(QRect(0,0, width(), height()), render_img, source);

	draw_element(painter);
    qDebug("New image position l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d), source=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
           view_rect.right(), view_rect.bottom(), size().width(), size().height(), source.width(), source.height());
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
    case Qt::Key_0:
        bk_layer = 0;
        break;
    case Qt::Key_1:
        if (gcst.num_layer() > 1)
            bk_layer = 1;
        break;
    case Qt::Key_2:
        if (gcst.num_layer() > 2)
            bk_layer = 2;
        break;
    case Qt::Key_3:
        if (gcst.num_layer() > 3)
            bk_layer = 3;
        break;
    case Qt::Key_4:
        if (gcst.num_layer() > 4)
            bk_layer = 4;
        break;
    case Qt::Key_5:
        if (gcst.num_layer() > 5)
            bk_layer = 5;
        break;
    case Qt::Key_6:
        if (gcst.num_layer() > 6)
            bk_layer = 6;
        break;
    case Qt::Key_7:
        if (gcst.num_layer() > 7)
            bk_layer = 7;
        break;
    case Qt::Key_8:
        if (gcst.num_layer() > 8)
            bk_layer = 8;
        break;
    case Qt::Key_9:
        if (gcst.num_layer() > 9)
            bk_layer = 9;
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }

    qDebug("Key press, s=%5.2f, center=(%d,%d)", scale, center.x(), center.y());

    update();
}

void ConnectView::load_objects(string file_name)
{
    if (odb.load_objets(file_name) !=0)
        QMessageBox::about(this, "Load failed", "Please check file text");
    else
        update();
}

void ConnectView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mouse = gcst.pixel2bu(event->pos() * scale) + view_rect.topLeft();
    char s[200];
    emit mouse_change(mouse, QString(s));
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
        qDebug("render image l=%d,(%d,%d,%d,%d), im=(%d,%d)", layer,
               rect.left(), rect.top(),rect.right(), rect.bottom(), image.size().width(), image.size().height());
    render_rect = rect;
    render_img = image;
    render_bk_layer = layer;

    update();
}

void ConnectView::extract_cell_done(QSharedPointer<SearchResults> prst)
{
    for (int i = 0; i < prst->objs.size(); i++) {
        qInfo("Found cell (%d,%d) (%d,%d), dir=%d, prob=%f",
              prst->objs[i].p0.x(), prst->objs[i].p0.y(),
              prst->objs[i].p1.x(), prst->objs[i].p1.y(),
              prst->objs[i].type3, prst->objs[i].prob);
        ElementObj obj(prst->objs[i]);
        odb.add_object(obj);
    }
    QMessageBox::about(this, "Extract done", "Extract done");
}

void ConnectView::extract_wire_via_done(QSharedPointer<SearchResults> prst)
{
    for (int i = 0; i < prst->objs.size(); i++) {
        ElementObj obj(prst->objs[i]);
        odb.add_object(obj);
    }
    QMessageBox::about(this, "Extract done", "Extract done");
}

void ConnectView::train(bool cell_train, int i1, int i2, int i3, int i4, float f1, float f2, float f3)
{
    if (cell_train) {
        qInfo("Accept cell train: p1=%f, p2=%f, p3=%f", f1, f2, f3);
        QPoint p0(1551572, 255590), p1(1565716, 259494);
        emit train_cell(1, 255, 255, 255, POWER_UP_L, QRect(p0, p1), f1, f2, f3);
    }    
}

void ConnectView::extract(bool cell_train, int i1, int i2, int i3, int i4, float f1, float f2, float f3)
{
    if (cell_train) {
        qInfo("Accept cell extract: p1=%f, p2=%f, p3=%f", f1, f2, f3);
        SearchRects * sr = new SearchRects;
        sr->dir.push_back(POWER_UP | POWER_DOWN);
        sr->rects.push_back(QRect(1300000, 200000, 500000, 500000));
        emit extract_cell(1, 255, 255, 255, QSharedPointer<SearchRects>(sr), f1, f2, f3);
    } else {
        qInfo("Accept wire extract: p1=%f, p2=%f, p3=%f", f1, f2, f3);
        VWSearchRequest * preq = new VWSearchRequest;
        preq->lpa.push_back(LayerParam(2, 10, 9,
            RULE_NO_LOOP | RULE_NO_UCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA,
            16, 0.5, 0.5, 2));
        preq->lpa.push_back(LayerParam(3, 12, 10,
            RULE_NO_LOOP | RULE_NO_UCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA,
            16, 0.5, 0.5, 2));
        preq->lpa.push_back(LayerParam(4, 12, 10,
            RULE_NO_LOOP | RULE_NO_UCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA,
            16, 0.5, 0.5, 2));
        emit extract_wire_via(QSharedPointer<VWSearchRequest>(preq), QRect(1430000, 586000, 65536, 65536));
    }
}
