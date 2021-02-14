#include "connectview.h"
#include <QPainter>
#include <QMessageBox>
#include <QGuiApplication>
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
    license = ""; //"0A(12)@20";
	screen_2= QRect(300, 200, 300, 200);
	view_rect_2 = QRect(screen_2.x(), screen_2.y(), screen_2.width(), screen_2.height());
	center_2 = view_rect_2.center(); 
	dia.resize(15, 10);
	wide_x.resize(10, 5);
	wide_y.resize(10, 5);
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
	view_rect_2 = QRect(screen_2.x(), screen_2.y(), screen_2.width(), screen_2.height());
	center_2 = view_rect_2.center();
    emit render_bkimg(prj_file, license, bk_layer, view_rect, size(), RETURN_WHEN_PART_READY, this, true);
}

string ConnectView::get_prj_file()
{
	return prj_file;
}

int ConnectView::get_current_layer()
{
	return bk_layer;
}

void ConnectView::get_dia(vector<int> & dia_)
{
	dia_ = dia;
}

void ConnectView::set_dia(const vector<int> & dia_)
{
	dia = dia_;
}

void ConnectView::get_wide_xy(vector<int> & wide_x_, vector<int> & wide_y_)
{
	wide_x_ = wide_x;
	wide_y_ = wide_y;
}

void ConnectView::set_wide_xy(const vector<int> & wide_x_, const vector<int> & wide_y_)
{
	wide_x = wide_x_;
	wide_y = wide_y_;
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
        if (obj.prob > 0.9)
            painter.setPen(QPen(Qt::blue, 1));
        else
            painter.setPen(QPen(Qt::yellow, 1));
        if (obj.type3 == bk_layer)
            painter.drawLine(p0, p1);        
        break;
    case OBJ_POINT:
        if (obj.prob > 0.9)
            painter.setPen(QPen(Qt::green, 1));
        else
            painter.setPen(QPen(Qt::yellow, 1));
		if (obj.type3 == bk_layer) {
			QPoint tl(p0.x() - obj.p1.x() / 2, p0.y() - obj.p1.y() / 2);
			QPoint br(p0.x() - obj.p1.x() / 2 + obj.p1.x(), p0.y() - obj.p1.y() / 2 + obj.p1.y());
			painter.drawEllipse(p0, 5, 5);
		}
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

	odb.get_objects(OBJ_LINE, LINE_WIRE_AUTO_EXTRACT_MASK | LINE_WIRE_AUTO_EXTRACT1_MASK, view_rect, rst);
    bool draw_wire_end = (view_rect.height() < 800);
    painter.setBrush(QBrush(Qt::NoBrush));
    if (rst.size()!=0)
        qDebug("draw %d lines", rst.size());
    for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type3 == bk_layer) {
			if (rst[i]->type2 != LINE_WIRE_AUTO_EXTRACT1) {
				if (rst[i]->prob > 0.9)
					painter.setPen(QPen(Qt::blue, 1));
				else
					painter.setPen(QPen(Qt::yellow, 1));
			}
			else {
				int r = (rst[i]->un.attach ^ rst[i]->un.attach >> 24) & 0xff;
				int g = (rst[i]->un.attach >> 8 ^ rst[i]->un.attach >> 32) & 0xff;
				int b = (rst[i]->un.attach >> 16 ^ rst[i]->un.attach >> 40) & 0xff;
				if (r >= g && r >= b)
					r |= 128; //make it bright
				if (g >= r && g >= b)
					g |= 128;
				if (b >= r && b >= g)
					b |= 128;
				painter.setPen(QPen(QColor(r, g, b), 1));
			}				
            QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
            QPoint p1(width() * (rst[i]->p1.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p1.y() - view_rect.top()) / view_rect.height());
            painter.drawLine(p0, p1);
            if (draw_wire_end)
                painter.drawEllipse(p1, 2, 2);
        }
    }

    odb.get_objects(OBJ_POINT, POINT_VIA_AUTO_EXTRACT_MASK, view_rect, rst);
    painter.setPen(QPen(Qt::green, 2));
    painter.setBrush(QBrush(Qt::NoBrush));
    if (rst.size()!=0)
        qDebug("draw %d via extract", rst.size());
    for (unsigned i = 0; i < rst.size(); i++) {
        if (rst[i]->type3 == bk_layer) {
            QPoint p0(width() * (rst[i]->p0.x() - view_rect.left()) / view_rect.width(),
                height()* (rst[i]->p0.y() - view_rect.top()) / view_rect.height());
            painter.drawEllipse(p0, 5, 5);
        }
    }

	odb.get_objects(OBJ_POINT, POINT_VIA_AUTO_EXTRACT1_MASK | POINT_NORMAL_VIA0_MASK | POINT_NO_VIA_MASK, view_rect, rst);
	painter.setPen(QPen(Qt::green, 2));
	painter.setBrush(QBrush(Qt::NoBrush));
	if (rst.size() != 0)
		qDebug("draw %d vias", rst.size());
	for (unsigned i = 0; i < rst.size(); i++) {
		if (rst[i]->type3 == bk_layer) {
			if (rst[i]->type2 == POINT_NO_VIA)
				painter.setPen(QPen(Qt::red, 2));
			else
				painter.setPen(QPen(Qt::green, 2));
			QPoint p0(width() * (rst[i]->p0.x() - view_rect.left() - rst[i]->p1.x() / 2) / view_rect.width(),
				height()* (rst[i]->p0.y() - view_rect.top() - rst[i]->p1.y() / 2) / view_rect.height());
			QPoint p1(width() * (rst[i]->p0.x() - view_rect.left() + rst[i]->p1.x() / 2) / view_rect.width(),
				height()* (rst[i]->p0.y() - view_rect.top() + rst[i]->p1.y() / 2) / view_rect.height());
			painter.drawEllipse(QRect(p0, p1));
		}
	}

	odb.get_objects(OBJ_POINT, POINT_WIRE_INSU_MASK | POINT_WIRE_MASK | POINT_INSU_MASK, view_rect, rst);
	painter.setPen(QPen(Qt::green, 2));
	painter.setBrush(QBrush(Qt::NoBrush));
	if (rst.size() != 0)
		qDebug("draw %d train objs", rst.size());
	for (unsigned i = 0; i < rst.size(); i++) {
		if (rst[i]->type3 == bk_layer) {
			painter.setPen(QPen(Qt::green, 2));
			if (rst[i]->type2 == POINT_WIRE_INSU)
				painter.drawLine(rst[i]->p0, rst[i]->p1);
			else
				painter.drawRect(rst[i]->p0.x() - 1, rst[i]->p0.y() - 1, 3, 3);
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
		view_rect = QRect(center - ce, s);
		if (view_rect.width() > pcst->tot_width_pixel())
			view_rect.adjust(view_rect.width() - pcst->tot_width_pixel(), 0, 0, 0);
		if (view_rect.height() > pcst->tot_height_pixel())
			view_rect.adjust(0, view_rect.height() - pcst->tot_height_pixel(), 0, 0);
		if (view_rect.left() < 0)
			view_rect.moveLeft(0);
		if (view_rect.right() >= pcst->tot_width_pixel())
			view_rect.moveRight(pcst->tot_width_pixel() - 1);
		if (view_rect.top() < 0)
			view_rect.moveTop(0);
		if (view_rect.bottom() >= pcst->tot_height_pixel())
			view_rect.moveBottom(pcst->tot_height_pixel() - 1);
		Q_ASSERT(view_rect.left() >= 0 && view_rect.right() <= pcst->tot_width_pixel() &&
			view_rect.top() >= 0 && view_rect.bottom() <= pcst->tot_height_pixel());
		if (ds != DISPLAY1) {
			QPoint ce2(screen_2.size().width()*scale_2 / 2, screen_2.size().height()*scale_2 / 2);
			QSize s2(screen_2.size().width()*scale_2, screen_2.size().height()*scale_2);
			view_rect_2 = QRect(center_2 - ce2, s2);
			if (view_rect_2.width() > pcst->tot_width_pixel())
				view_rect_2.adjust(view_rect_2.width() - pcst->tot_width_pixel(), 0, 0, 0);
			if (view_rect_2.height() > pcst->tot_height_pixel())
				view_rect_2.adjust(0, view_rect_2.height() - pcst->tot_height_pixel(), 0, 0);
			if (view_rect_2.left() < 0)
				view_rect_2.moveLeft(0);
			if (view_rect_2.right() > pcst->tot_width_pixel())
				view_rect_2.moveRight(pcst->tot_width_pixel());
			if (view_rect_2.top() < 0)
				view_rect_2.moveTop(0);
			if (view_rect_2.bottom() > pcst->tot_height_pixel())
				view_rect_2.moveBottom(pcst->tot_height_pixel());
			Q_ASSERT(view_rect_2.left() >= 0 && view_rect_2.right() <= pcst->tot_width_pixel() &&
				view_rect_2.top() >= 0 && view_rect_2.bottom() <= pcst->tot_height_pixel());
		}
	}

    if (!render_rect.contains(view_rect) || render_bk_layer !=bk_layer) {
        qDebug("request image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.width(), view_rect.height(), size().width(), size().height());
        emit render_bkimg(prj_file, license, bk_layer, pcst->pixel2bu(view_rect), size(), RETURN_WHEN_PART_READY, this, true);
        return;
    }

	if ((render_rect.width() > view_rect.width() * 3 && view_rect.width() >= view_rect.height() && render_rect.width() > pcst->img_block_w() *3) ||
		(render_rect.height() > view_rect.height() * 3 && view_rect.width() < view_rect.height() && render_rect.height() > pcst->img_block_h() * 3)) {
        qDebug("request clear image l=%d,vr=(%d,%d,%d,%d), screen=(%d,%d)", bk_layer, view_rect.left(), view_rect.top(),
               view_rect.width(), view_rect.height(), size().width(), size().height());
        emit render_bkimg(prj_file, license, bk_layer, pcst->pixel2bu(view_rect), size(), RETURN_WHEN_PART_READY, this, true);
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
            emit render_bkimg(prj_file, license, bk_layer_2, pcst->pixel2bu(view_rect_2), screen_2.size(), RETURN_EXACT_MATCH, &screen_2, false);
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
            view_rect.moveLeft(max(0, view_rect.left() - step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.width() * step_para / 10;
            view_rect_2.moveLeft(max(0, view_rect_2.left() - step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Up:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.height() * step_para / 10;
            view_rect.moveTop(max(0, view_rect.top() - step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.height() * step_para / 10;
            view_rect_2.moveTop(max(0, view_rect_2.top() - step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Right:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.width() * step_para / 10;
            view_rect.moveRight(min(pcst->tot_width_pixel(), view_rect.right() + step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.width() * step_para / 10;
            view_rect_2.moveRight(min(pcst->tot_width_pixel(), view_rect_2.right() + step));
			center_2 = view_rect_2.center();
			renew_img_2 = true;
		}
        break;
    case Qt::Key_Down:
		if (ds == DISPLAY1 || ds == DISPLAY12_OP1) {
			step = view_rect.height() * step_para / 10;
            view_rect.moveBottom(min(pcst->tot_height_pixel(), view_rect.bottom() + step));
			center = view_rect.center();
		}
		if (ds == DISPLAY12_OP2) {
			step = view_rect_2.height() * step_para / 10;
            view_rect_2.moveBottom(min(pcst->tot_height_pixel(), view_rect_2.bottom() + step));
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
    case Qt::Key_P:
        emit render_bkimg(prj_file, license, bk_layer, pcst->pixel2bu(view_rect), size(), PRINT_SCREEN_NO_RETURN, this, true);
		break;

	case Qt::Key_T:
		/*
		QImage img;
		QRect ret;
		emit render_bkimg_blocking(prj_file, bk_layer, QRect(200, 200, 12700, 260), QSize(12700, 260), RETURN_EXACT_MATCH, ret, img);
		char s[50];
		sprintf(s, "sc_%d_%d_M", view_rect.top() / pcst->img_block_h(), view_rect.left() / pcst->img_block_w());
		QString filename(s);
		filename.append('0' + bk_layer);
		filename.append(".jpg");
		img.save(filename, "JPG");
		return; */
		break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }

    qDebug("Key press=%d, s=%5.2f, c=(%d,%d), s2=%5.2f, c2=(%d,%d)", e->key(), scale, center.x(), center.y(), scale_2,
		center_2.x(), center_2.y());

	if (renew_img_2) {
		qDebug("request img2, vr2=(%d,%d,%d,%d), screen=(%d,%d)", view_rect_2.left(), view_rect_2.top(),
			view_rect_2.width(), view_rect_2.height(), screen_2.size().width(), screen_2.size().height());
        emit render_bkimg(prj_file, license, bk_layer_2, pcst->pixel2bu(view_rect_2), screen_2.size(), RETURN_EXACT_MATCH, &screen_2, false);
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

void ConnectView::set_license(string _license)
{
    license = _license;
}

void ConnectView::goto_xy(int x, int y, int layer)
{
    center.setX(x);
    center.setY(y);
    if (layer > 0 && pcst->num_layer() > layer) {
        bk_layer = layer;
        bk_layer_2 = layer;
    }
    qDebug("Goto s=%5.2f c=(%d,%d), l=%d", scale, center.x(), center.y(), bk_layer);
    update();
}

void ConnectView::clear_objs()
{
    odb.clear_all();
    update();
}

void ConnectView::clear_wire_via()
{
	odb.del_objects(OBJ_LINE, LINE_WIRE_AUTO_EXTRACT_MASK | LINE_WIRE_AUTO_EXTRACT1_MASK, 1.1);
    odb.del_objects(OBJ_POINT, POINT_VIA_AUTO_EXTRACT_MASK, 1.1);
    update();
}

void ConnectView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mp = event->pos() * scale + view_rect.topLeft();
    char s[200] = "";

    if (!render_img.isNull()) {
        QPoint xy((double) render_img.width() * (mp.x() - render_rect.left()) / render_rect.width(),
              (double) render_img.height() * (mp.y() - render_rect.top()) / render_rect.height());
        sprintf(s, " c=%x", render_img.pixel(xy));
    }
    if (ms.state == CHOOSE_ANO_POINT) {
        ms.draw_obj.p1 = mp;
        update();
    }
    emit mouse_change(mp, QString(s));
}

void ConnectView::mousePressEvent(QMouseEvent *event)
{
    QPoint mp = event->pos() * scale + view_rect.topLeft();
	vector<QPoint> org;
	org.push_back(mp);
    if (QGuiApplication::queryKeyboardModifiers() == Qt::ControlModifier) {
        single_wire_extract(bk_layer, 5, 128, 0, 39, 0, org, -1, -1, 0xf);
        return;
    }
    if (ms.state == CREATE_NEW_OBJ) {
		if (ms.type != OBJ_POINT) {
			ms.draw_obj.type = ms.type;
			ms.draw_obj.type2 = ms.type2;
			ms.draw_obj.type3 = bk_layer;
			ms.draw_obj.p0 = mp;
			ms.state = CHOOSE_ANO_POINT;
		}
		else {
			if (ms.type2 == POINT_NORMAL_VIA0)
				ml_via_train(bk_layer, dia[bk_layer], dia[bk_layer] + 1, 1, mp);
			if (ms.type2 == POINT_NO_VIA)
				ml_via_train(bk_layer, dia[bk_layer], dia[bk_layer] + 1, 0, mp);
			if (ms.type2 == POINT_WIRE_INSU)
				ml_wire_train(bk_layer, POINT_WIRE_INSU, mp);
		}
    }
}

void ConnectView::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint mp = event->pos() * scale + view_rect.topLeft();
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
	QRect rect1 = pcst->bu2pixel(rect);
	if (this != view) {
		if (rect1 != view_rect_2 || layer != bk_layer_2 || screen != screen_2.size()) {
			qWarning("update view2 receive obsolete image");
			return;
		}
		else
			qDebug("render image2 done, l=%d,(%d,%d,%d,%d), im=(%d,%d)", layer,
			rect1.left(), rect1.top(), rect1.width(), rect1.height(), image.size().width(), image.size().height());
		render_img_2 = image;
	}
	else {
		if (!rect1.contains(view_rect) || layer != bk_layer || screen != size()) {
			qWarning("update view receive obsolete image");
			return;
		}
		else
			qDebug("render image done, l=%d,(%d,%d,%d,%d), im=(%d,%d)", layer,
			rect1.left(), rect1.top(), rect1.width(), rect1.height(), image.size().width(), image.size().height());
		render_rect = rect1;
		render_img = image;
		render_bk_layer = layer;
	}
    update();
}

void ConnectView::extract_cell_done(QSharedPointer<SearchResults> prst)
{
    for (int i = 0; i < prst->objs.size(); i++) {        
        ElementObj obj(prst->objs[i]);
		obj.p0 = pcst->bu2pixel(obj.p0);
		obj.p1 = pcst->bu2pixel(obj.p1);
		qInfo("Found cell (%d,%d) (%d,%d), dir=%d, prob=%f",
			obj.p0.x(), obj.p0.y(), obj.p1.x(), obj.p1.y(),	obj.type3, obj.prob);
        odb.add_object(obj);
    }
    QMessageBox::about(this, "Extract cell done", "Extract cell done");
}

void ConnectView::extract_single_wire_done(QSharedPointer<SearchResults> prst)
{
    for (int i = 0; i < prst->objs.size(); i++) {
        ElementObj obj(prst->objs[i]);
        obj.p0 = pcst->bu2pixel(obj.p0);
        obj.p1 = pcst->bu2pixel(obj.p1);
        odb.add_object(obj);
    }
    update();
}

void ConnectView::extract_wire_via_done(QSharedPointer<SearchResults> prst)
{
    for (int i = 0; i < prst->objs.size(); i++) {
        ElementObj obj(prst->objs[i]);
		obj.p0 = pcst->bu2pixel(obj.p0);
		obj.p1 = pcst->bu2pixel(obj.p1);
        odb.add_object(obj);
    }
    QMessageBox::about(this, "Extract wire done", "Extract wire done");
}

void ConnectView::train_vw_ml_done(QSharedPointer<SearchResults> prst)
{
	for (int i = 0; i < prst->objs.size(); i++) {
		ElementObj obj(prst->objs[i]);
		obj.p0 = pcst->bu2pixel(obj.p0);
		odb.add_object(obj);
	}
	update();
}

void ConnectView::del_vw_ml_done()
{

}

void ConnectView::get_train_vw_ml_done(QSharedPointer<SearchResults> prst)
{
	odb.del_objects(OBJ_POINT, POINT_WIRE_INSU_MASK | POINT_WIRE_MASK | POINT_INSU_MASK | POINT_NORMAL_VIA0_MASK, 2);
	train_vw_ml_done(prst);
}

void ConnectView::extract_ml_done(QSharedPointer<SearchResults> prst)
{
	update();
}

void ConnectView::cell_train(int i1, int i2, int i3, int i4, float f1, float f2, float f3)
{
    qInfo("Accept cell train: p1=%f, p2=%f, p3=%f", f1, f2, f3);
    vector<ElementObj*> tr;
    odb.get_objects(OBJ_AREA, AREA_LEARN_MASK, QRect(0, 0, pcst->tot_width_pixel(), pcst->tot_height_pixel()), tr);
    QPoint p0 = pcst->pixel2bu(tr[0]->p0);
    QPoint p1 = pcst->pixel2bu(tr[0]->p1);
    //emit train_cell(prj_file, 1, 255, 255, 255, POWER_UP_L, QRect(p0, p1), f1, f2, f3);
    emit train_cell(prj_file, license, tr[0]->type3, 255, 255, 255, POWER_LEFT_U, QRect(p0, p1), f1, f2, f3);
}

void ConnectView::cell_extract(int i1, int i2, int i3, int i4, float f1, float f2, float f3)
{    
    qInfo("Accept cell extract: p1=%f, p2=%f, p3=%f", f1, f2, f3);
    SearchRects * sr = new SearchRects;
    sr->dir.push_back(POWER_LEFT | POWER_RIGHT);
    vector<ElementObj*> er;
	odb.get_objects(OBJ_AREA, AREA_EXTRACT_MASK, QRect(0, 0, pcst->tot_width_pixel(), pcst->tot_height_pixel()), er);
    QPoint p0 = pcst->pixel2bu(er[0]->p0);
    QPoint p1 = pcst->pixel2bu(er[0]->p1);
    sr->rects.push_back(QRect(p0, p1));
    //emit extract_cell(prj_file, 1, 255, 255, 255, QSharedPointer<SearchRects>(sr), f1, f2, f3);
    emit extract_cell(prj_file, license, er[0]->type3, 255, 255, 255, QSharedPointer<SearchRects>(sr), f1, f2, f3);
}

void ConnectView::wire_extract(VWSearchRequest & vp, int opt)
{
	vector<ElementObj*> er;
	odb.get_objects(OBJ_AREA, AREA_EXTRACT_MASK, QRect(0, 0, pcst->tot_width_pixel(), pcst->tot_height_pixel()), er);
	QPoint p0 = pcst->pixel2bu(er[0]->p0);
	QPoint p1 = pcst->pixel2bu(er[0]->p1);
	VWSearchRequest * preq = new VWSearchRequest(vp);
    emit extract_wire_via(prj_file, license, QSharedPointer<VWSearchRequest>(preq), QRect(p0, p1), opt);
}

void ConnectView::single_wire_extract(int layer, int wmin, int wmax, int opt,
                         int gray_th, int channel, vector<QPoint> org, float cr, float cg, int shape_mask)
{
    opt =0;
    int s=0;
    int w=3000;
    while (view_rect.width()>w || view_rect.height()>w) {
        w=w<<1;
        s++;
    }
	vector<QPoint> * loc = new vector<QPoint>();
	for (auto &o : org)
		loc->push_back(pcst->pixel2bu(o));
    emit extract_single_wire(prj_file, license, layer, wmin, wmax, 5, opt, gray_th, channel, s, (void*) loc, cr, cg, shape_mask);
}

/*
 * Input layer
 * Input dmin, via diameter min
 * Input dmax, via diameter max
 * Input label, 1 means via, 0 means no via
 * Input v, Via location
 * Do via training
 */
void ConnectView::ml_via_train(int layer, int dmin, int dmax, int label, QPoint v)
{
	QPoint p = pcst->pixel2bu(v);
	emit train_via_ml(prj_file, license, layer, label, dmin, dmax, p.x(), p.y(),
		0, 0, 0, 0);
}

/*
Input layer
Input label
*/
void ConnectView::ml_wire_train(int layer, int label, QPoint v)
{
	QPoint p = pcst->pixel2bu(v);
	emit train_wire_ml(prj_file, license, layer, label, 6, 6, 6, p.x(), p.y(), 0);
}

/*
Input layer
Input d, via diameter
Input v, Via location
Del training via
*/
void ConnectView::ml_vw_del(int layer, int d, QPoint v)
{
	QPoint p = pcst->pixel2bu(v);
	emit del_vw_ml(prj_file, license, layer, d, p.x(), p.y());
}

/*
 * Input layer_min from which layer
 * Input layer_max to which layer
 * Input opt,
 * Extract via
 */
void ConnectView::vwml_extract(int layer, int opt)
{
	vector<ElementObj*> er;
	odb.get_objects(OBJ_AREA, AREA_EXTRACT_MASK, QRect(0, 0, pcst->tot_width_pixel(), pcst->tot_height_pixel()), er);
	QPoint p0 = pcst->pixel2bu(er[0]->p0);
	QPoint p1 = pcst->pixel2bu(er[0]->p1);
	QRect r(p0, p1);
	emit extract_ml(prj_file, license, layer, layer, QPolygon(r), wide_x[layer], wide_y[layer], 6, 0, 0, opt);
}

/*
Input layer
Input d, via diameter
Input v, Via location
Del training via
*/
void ConnectView::get_ml_obj(int layer)
{
	emit get_train_vw_ml(prj_file, license, layer);
}
