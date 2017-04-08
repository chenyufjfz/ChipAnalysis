#include "connectview.h"
#include <QPainter>
#include <QMessageBox>

extern ObjectDB odb;
//following parameter is for view_rect move
const int step_para =3;
const double min_scale = 0.03125;
const double max_scale = 128;

ConnectView::ConnectView(const char * prj_name, QWidget *parent) : QWidget(parent)
{    
    scale = 1;
	resize(600, 600);
    bk_layer = 0;
    render_bk_layer = 0;
    hide_element = false;
	pcst = RenderImage::register_new_window(this);
	SearchObject::register_new_window(this);
	prj_file = prj_name;
    ms.state = SELECT_EXIST;
    setAutoFillBackground(false);
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );
    setMouseTracking(true);
	ds = DISPLAY1;
	view_rect = QRect(0, 0, 128, 128);
	center = view_rect.center();
	bk_layer = 0;
	render_bk_layer = 255;
	bk_layer_2 = 0;
	scale_2 = 1;
	screen_2= QRect(300, 200, 300, 200);
	view_rect_2 = QRect(screen_2.x() * 32, screen_2.y() * 32, screen_2.width() * 32, screen_2.height() * 32);
	center_2 = view_rect_2.center(); 
#if 0
    ElementObj obj;
    obj.type = OBJ_AREA;
    obj.type2 = AREA_EXTRACT;
    obj.type3 = 1;
    obj.state = 0;
    obj.p0 = QPoint(1441792, 589824);
    obj.p1 = QPoint(1504560, 655000);
    odb.add_object(obj);
#endif
}

void ConnectView::set_prj_file(string _prj_file)
{
	prj_file = _prj_file;
	view_rect = QRect(0, 0, 128, 128);
	center = view_rect.center();
	bk_layer = 0;
	render_bk_layer = 255;
	ds = DISPLAY1;
	bk_layer_2 = 0;
	scale_2 = 1;
	screen_2 = QRect(300, 200, 300, 200);
	view_rect_2 = QRect(screen_2.x() * 32, screen_2.y() * 32, screen_2.width() * 32, screen_2.height() * 32);
	center_2 = view_rect_2.center();
	emit render_bkimg(prj_file, bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
}

string ConnectView::get_prj_file()
{
	return prj_file;
}

void ConnectView::draw_obj(ElementObj & obj, QPainter &painter)
{
    QPoint p0(width() * (obj.p0.x() - view_rect.left()) / view_rect.width(),
        height()* (obj.p0.y() - view_rect.top()) / view_rect.height());
    QPoint p1(width() * (obj.p1.x() - view_rect.left()) / view_rect.width(),
        height()* (obj.p1.y() - view_rect.top()) / view_rect.height());

    painter.setBrush(QBrush(Qt::NoBrush));
    switch (obj.type) {
    case OBJ_AREA:
        switch (obj.type2) {
        case AREA_CELL:        
            painter.setPen(QPen(Qt::red, 1));
            break;
        case AREA_LEARN:
            painter.setPen(QPen(Qt::blue, 1));
            break;
        case AREA_EXTRACT:
            painter.setPen(QPen(Qt::yellow, 1, Qt::DotLine));
            break;
        case AREA_CHECK_ERR:
            if (obj.type3 == bk_layer)
                painter.setPen(QPen(Qt::red, 1));
            break;
        }
        painter.drawRect(QRect(p0, p1));
        break;
    case OBJ_LINE:
        painter.setPen(QPen(Qt::blue, 1));
        if (obj.type3 == bk_layer)
            painter.drawLine(p0, p1);        
        break;
    case OBJ_POINT:
        painter.setPen(QPen(Qt::green, 1, Qt::DotLine));
        if (obj.type3 == bk_layer)
            painter.drawEllipse(p0, 9, 9);        
        break;
    }
}

void ConnectView::draw_element(QPainter &painter)
{
	vector<ElementObj*> rst;
    odb.get_objects(OBJ_AREA, AREA_CELL_MASK | AREA_LEARN_MASK | AREA_EXTRACT_MASK | AREA_CHECK_ERR_MASK, view_rect, rst);
	painter.setBrush(QBrush(Qt::NoBrush));
	for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type2==AREA_CHECK_ERR && rst[i]->type3 != bk_layer)
            continue;
		switch (rst[i]->type2) {
		case AREA_CELL:        
			painter.setPen(QPen(Qt::red, 1));
			break;
		case AREA_LEARN:
			painter.setPen(QPen(Qt::blue, 1));
			break;
		case AREA_EXTRACT:
            painter.setPen(QPen(Qt::yellow, 1, Qt::DotLine));
            break;
        case AREA_CHECK_ERR:
            painter.setPen(QPen(QColor(200 * rst[i]->prob, 0, 0), 1));
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
    painter.setBrush(QBrush(Qt::NoBrush));
    if (rst.size()!=0)
        qDebug("draw %d lines", rst.size());
    for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type3 == bk_layer) {
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
        if (rst[i]->type3 == bk_layer) {
            QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
            painter.drawEllipse(p0, 9, 9);
        }
    }

    if (ms.state == CHOOSE_ANO_POINT)
        draw_obj(ms.draw_obj, painter);
}

void ConnectView::paintEvent(QPaintEvent *)
{
	if (pcst->num_layer() == 0) {//prj is not opened yet, hard code view_rect, bk_layer to force open prj file
		view_rect = QRect(0, 0, 128, 128);
		bk_layer = 0;
		render_bk_layer = 255;
	}
	else {
		QPoint ce(size().width()*scale / 2, size().height()*scale / 2);
		QSize s(size().width()*scale, size().height()*scale);
		view_rect = QRect(center - pcst->pixel2bu(ce), pcst->pixel2bu(s));
		if (view_rect.width() > pcst->tot_width_bu())
			view_rect.adjust(view_rect.width() - pcst->tot_width_bu(), 0, 0, 0);
		if (view_rect.height() > pcst->tot_height_bu())
			view_rect.adjust(0, view_rect.height() - pcst->tot_height_bu(), 0, 0);
		if (view_rect.left() < pcst->left_bound())
			view_rect.moveLeft(pcst->left_bound());
		if (view_rect.right() > pcst->right_bound())
			view_rect.moveRight(pcst->right_bound());
		if (view_rect.top() < pcst->top_bound())
			view_rect.moveTop(pcst->top_bound());
		if (view_rect.bottom() > pcst->bottom_bound())
			view_rect.moveBottom(pcst->bottom_bound());
		Q_ASSERT(view_rect.left() >= pcst->left_bound() && view_rect.right() <= pcst->right_bound() &&
			view_rect.top() >= pcst->top_bound() && view_rect.bottom() <= pcst->bottom_bound());
		if (ds != DISPLAY1) {
			QPoint ce2(screen_2.size().width()*scale_2 / 2, screen_2.size().height()*scale_2 / 2);
			QSize s2(screen_2.size().width()*scale_2, screen_2.size().height()*scale_2);
			view_rect_2 = QRect(center_2 - pcst->pixel2bu(ce2), pcst->pixel2bu(s2));
			if (view_rect_2.width() > pcst->tot_width_bu())
				view_rect_2.adjust(view_rect_2.width() - pcst->tot_width_bu(), 0, 0, 0);
			if (view_rect_2.height() > pcst->tot_height_bu())
				view_rect_2.adjust(0, view_rect_2.height() - pcst->tot_height_bu(), 0, 0);
			if (view_rect_2.left() < pcst->left_bound())
				view_rect_2.moveLeft(pcst->left_bound());
			if (view_rect_2.right() > pcst->right_bound())
				view_rect_2.moveRight(pcst->right_bound());
			if (view_rect_2.top() < pcst->top_bound())
				view_rect_2.moveTop(pcst->top_bound());
			if (view_rect_2.bottom() > pcst->bottom_bound())
				view_rect_2.moveBottom(pcst->bottom_bound());
			Q_ASSERT(view_rect_2.left() >= pcst->left_bound() && view_rect_2.right() <= pcst->right_bound() &&
				view_rect_2.top() >= pcst->top_bound() && view_rect_2.bottom() <= pcst->bottom_bound());
		}
	}

    if (!render_rect.contains(view_rect) || render_bk_layer !=bk_layer) {
        qDebug("request image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.width(), view_rect.height(), size().width(), size().height());
        emit render_bkimg(prj_file, bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
        return;
    }

	if ((render_rect.width() > view_rect.width() * 3 && view_rect.width() >= view_rect.height() && render_rect.width() > 0x18000) ||
		(render_rect.height() > view_rect.height() * 3 && view_rect.width() < view_rect.height() && render_rect.height() > 0x18000)) {
        qDebug("request clear image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.width(), view_rect.height(), size().width(), size().height());
		emit render_bkimg(prj_file, bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
    }
    QPainter painter(this);

    QRect source((double) render_img.width() * (view_rect.left() - render_rect.left()) / render_rect.width(),
                 (double) render_img.height()* (view_rect.top() - render_rect.top()) /render_rect.height(),
                 (double) render_img.width() * view_rect.width() /render_rect.width(),
                 (double) render_img.height()* view_rect.height() /render_rect.height());
    painter.drawImage(QRect(0,0, width(), height()), render_img, source);
	if (ds != DISPLAY1) {
		if (render_img_2.isNull()) {
			qDebug("img_2 null, request img2, vr2=(%d,%d,%d,%d), screen=(%d,%d)", view_rect_2.left(), view_rect_2.top(), 
				view_rect_2.width(), view_rect_2.height(), screen_2.size().width(), screen_2.size().height());
			emit render_bkimg(prj_file, bk_layer_2, view_rect_2, screen_2.size(), RETURN_EXACT_MATCH, &screen_2, false);
		}
		else
			painter.drawImage(screen_2.topLeft(), render_img_2);
	}
    if (!hide_element)
        draw_element(painter);
    qDebug("New image position l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d), source=(%d,%d,%d,%d)", bk_layer, 
		view_rect.left(), view_rect.top(), view_rect.width(), view_rect.height(), 
		size().width(), size().height(), source.left(), source.top(), source.width(), source.height());
}

void ConnectView::keyPressEvent(QKeyEvent *e)
{
    int step;
	bool renew_img_2 = false;
    switch (e->key()) {
    case Qt::Key_Left:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.width() * step_para / 10;
			view_rect.moveLeft(max(pcst->left_bound(), view_rect.left() - step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.width() * step_para / 10;
			view_rect_2.moveLeft(max(pcst->left_bound(), view_rect_2.left() - step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Up:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.height() * step_para / 10;
			view_rect.moveTop(max(pcst->top_bound(), view_rect.top() - step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.height() * step_para / 10;
			view_rect_2.moveTop(max(pcst->top_bound(), view_rect_2.top() - step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Right:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.width() * step_para / 10;
			view_rect.moveRight(min(pcst->right_bound(), view_rect.right() + step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.width() * step_para / 10;
			view_rect_2.moveRight(min(pcst->right_bound(), view_rect_2.right() + step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Down:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.height() * step_para / 10;
			view_rect.moveBottom(min(pcst->bottom_bound(), view_rect.bottom() + step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.height() * step_para / 10;
			view_rect_2.moveBottom(min(pcst->bottom_bound(), view_rect_2.bottom() + step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_PageUp:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			if (scale < max_scale)
				scale = scale * 2;
		}
		if (ds == DISPLAY12_OP2) {
			if (scale_2 < max_scale) {
				scale_2 = scale_2 * 2;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_PageDown:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			if (scale > min_scale)
				scale = scale / 2;
		}
		if (ds == DISPLAY12_OP2) {
			if (scale_2 > min_scale) {
				scale_2 = scale_2 / 2;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_0:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
			bk_layer = 0;
		if (ds == DISPLAY12_OP2 && bk_layer_2 != 0) {
			bk_layer_2 = 0;
			renew_img_2 = true;
		}
        break;
    case Qt::Key_1:
		if (pcst->num_layer() > 1) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 1;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 1) {
				bk_layer_2 = 1;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_2:
		if (pcst->num_layer() > 2) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 2;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 2) {
				bk_layer_2 = 2;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_3:
		if (pcst->num_layer() > 3) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 3;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 3) {
				bk_layer_2 = 3;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_4:
		if (pcst->num_layer() > 4) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 4;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 4) {
				bk_layer_2 = 4;
				renew_img_2 = true;
			}
		}            
        break;
    case Qt::Key_5:
		if (pcst->num_layer() > 5) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 5;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 5) {
				bk_layer_2 = 5;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_6:
		if (pcst->num_layer() > 6) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 6;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 6) {
				bk_layer_2 = 6;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_7:
		if (pcst->num_layer() > 7) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 7;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 7) {
				bk_layer_2 = 7;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_8:
		if (pcst->num_layer() > 8) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 8;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 8) {
				bk_layer_2 = 8;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_9:
		if (pcst->num_layer() > 9) {
			if (ds == DISPLAY1 || ds == DISPLAY12_OP1)
				bk_layer = 9;
			if (ds == DISPLAY12_OP2 && bk_layer_2 != 9) {
				bk_layer_2 = 9;
				renew_img_2 = true;
			}
		}
        break;
    case Qt::Key_H:
        hide_element = !hide_element;
        break;
	case Qt::Key_D:
		if (ds == DISPLAY1)
			ds = DISPLAY12_OP2;
		else
			if (ds == DISPLAY12_OP2)
				ds = DISPLAY12_OP1;
			else
				ds = DISPLAY1;
		break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }

	qDebug("Key press, s=%5.2f, c=(%d,%d), s2=%5.2f, c2=(%d,%d)", scale, center.x(), center.y(), scale_2, 
		center_2.x(), center_2.y());

	if (renew_img_2) {
		qDebug("request img2, vr2=(%d,%d,%d,%d), screen=(%d,%d)", view_rect_2.left(), view_rect_2.top(),
			view_rect_2.width(), view_rect_2.height(), screen_2.size().width(), screen_2.size().height());
		emit render_bkimg(prj_file, bk_layer_2, view_rect_2, screen_2.size(), RETURN_EXACT_MATCH, &screen_2, false);
	} 
	else
		update();
}

void ConnectView::load_objects(string file_name)
{
    if (odb.load_objets(file_name) !=0)
        QMessageBox::about(this, "Load failed", "Please check file text");
    else
        update();
}

void ConnectView::set_mark(unsigned char type, unsigned char type2)
{
    ms.state = CREATE_NEW_OBJ;
    ms.type = type;
    ms.type2 = type2;
    setFocus();
}

void ConnectView::clear_objs()
{
    odb.clear_all();
    update();
}

void ConnectView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mp = pcst->pixel2bu(event->pos() * scale) + view_rect.topLeft();
    char s[200] = "";
    if (ms.state == CHOOSE_ANO_POINT) {
        ms.draw_obj.p1 = mp;
        update();
    }
    emit mouse_change(mp, QString(s));
}

void ConnectView::mousePressEvent(QMouseEvent *event)
{
    QPoint mp = pcst->pixel2bu(event->pos() * scale) + view_rect.topLeft();
    if (ms.state == CREATE_NEW_OBJ) {
        ms.draw_obj.type = ms.type;
        ms.draw_obj.type2 = ms.type2;
        ms.draw_obj.type3 = bk_layer;
        ms.draw_obj.p0 = mp;
        ms.state = CHOOSE_ANO_POINT;
    }
}

void ConnectView::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint mp = pcst->pixel2bu(event->pos() * scale) + view_rect.topLeft();
    if (ms.state == CHOOSE_ANO_POINT) {
        ms.draw_obj.p1 = mp;
        odb.add_object(ms.draw_obj);
        ms.state = CREATE_NEW_OBJ;
        update();
    }
}

void ConnectView::server_connected()
{
}

void ConnectView::server_disconnected()
{
}

void ConnectView::render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
	QImage image, bool finish, const void * view)
{
	if (this != view) {
		if (rect != view_rect_2 || layer != bk_layer_2 || screen != screen_2.size()) {
			qWarning("update view2 receive obsolete image");
			return;
		}
		else
			qDebug("render image2 done, l=%d,(%d,%d,%d,%d), im=(%d,%d)", layer,
			rect.left(), rect.top(), rect.width(), rect.height(), image.size().width(), image.size().height());
		render_img_2 = image;
	}
	else {
		if (!rect.contains(view_rect) || layer != bk_layer || screen != size()) {
			qWarning("update view receive obsolete image");
			return;
		}
		else
			qDebug("render image done, l=%d,(%d,%d,%d,%d), im=(%d,%d)", layer,
			rect.left(), rect.top(), rect.width(), rect.height(), image.size().width(), image.size().height());
		render_rect = rect;
		render_img = image;
		render_bk_layer = layer;
	}
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
        //emit train_cell(1, 255, 255, 255, POWER_UP_L, QRect(p0, p1), f1, f2, f3);
        vector<ElementObj*> tr;
        odb.get_objects(OBJ_AREA, AREA_LEARN_MASK, pcst->bound_rect_bu(), tr);
        p0 = tr[0]->p0;
        p1 = tr[0]->p1;
        emit train_cell(prj_file, 1, 255, 255, 255, POWER_UP_L, QRect(p0, p1), f1, f2, f3);
    }    
}

void ConnectView::extract(bool cell_train, int i1, int i2, int i3, int i4, float f1, float f2, float f3)
{
    if (cell_train) {
        qInfo("Accept cell extract: p1=%f, p2=%f, p3=%f", f1, f2, f3);
        SearchRects * sr = new SearchRects;
        sr->dir.push_back(POWER_UP | POWER_DOWN);
        //sr->rects.push_back(QRect(1300000, 200000, 500000, 500000));
        //emit extract_cell(1, 255, 255, 255, QSharedPointer<SearchRects>(sr), f1, f2, f3);
        vector<ElementObj*> er;
        odb.get_objects(OBJ_AREA, AREA_EXTRACT_MASK, pcst->bound_rect_bu(), er);
        QPoint p0 = er[0]->p0;
        QPoint p1 = er[0]->p1;
        sr->rects.push_back(QRect(p0, p1));
        emit extract_cell(prj_file, 1, 255, 255, 255, QSharedPointer<SearchRects>(sr), f1, f2, f3);
    } else {
        qInfo("Accept wire extract: p1=%f, p2=%f, p3=%f", f1, f2, f3);
        VWSearchRequest * preq = new VWSearchRequest;
        preq->lpa.push_back(LayerParam(1, 4, 9,
            RULE_END_WITH_VIA, RULE_END_WITH_VIA,
            16, 0.5, 0.5, 1, 0));
        preq->lpa.push_back(LayerParam(2, 10, 9,
            RULE_NO_LOOP | RULE_NO_HCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA | RULE_EXTEND_VIA_OVERLAP | RULE_NO_ADJ_VIA_CONN,
            RULE_NO_hCONN, 16, 0.5, 0.5, 2, 0));
        preq->lpa.push_back(LayerParam(3, 12, 10,
            RULE_NO_LOOP | RULE_NO_HCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA | RULE_EXTEND_VIA_OVERLAP | RULE_NO_ADJ_VIA_CONN,
            RULE_NO_hCONN, 16, 0.5, 0.5, 2, 0));
        preq->lpa.push_back(LayerParam(4, 12, 10,
            RULE_NO_LOOP | RULE_NO_HCONN | RULE_NO_TT_CONN | RULE_END_WITH_VIA | RULE_EXTEND_VIA_OVERLAP | RULE_NO_ADJ_VIA_CONN,
            RULE_NO_hCONN, 16, 0.5, 0.5, 1, 0));
        //QRect(1430000, 586000, 65536, 65536), QRect(1450000, 600000, 65536, 65536)
        //QRect(1450000, 586000, 65536, 65536)
        //emit extract_wire_via(QSharedPointer<VWSearchRequest>(preq), QRect(1430000, 600000, 65536, 65536));
        vector<ElementObj*> er;
        odb.get_objects(OBJ_AREA, AREA_EXTRACT_MASK, pcst->bound_rect_bu(), er);
        QPoint p0 = er[0]->p0;
        QPoint p1 = er[0]->p1;
        emit extract_wire_via(prj_file, QSharedPointer<VWSearchRequest>(preq), QRect(p0, p1));
    }
}
