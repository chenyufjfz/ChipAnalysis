#include "renderimage.h"
#include <QPainter>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <QtCore/QSharedPointer>

#define CACHE_SIZE (1<<29)
#define PREV_IMG_NUM 5
BkImgRoMgr bkimg_faty;

PrjConst::PrjConst()
{
	reset();
}

void PrjConst::set(int img_block_w, int img_block_h, int num_block_x, int num_block_y, int num_layer, int max_scale)
{
	_img_block_w = img_block_w; //server side image width in pixel
	_img_block_h = img_block_h; //server side image height in pixel
	_num_block_x = num_block_x;
	_num_block_y = num_block_y;
	_num_layer = num_layer;
	_max_scale = max_scale;
	_pixel_bu = 32768 / _img_block_w;
}

void PrjConst::reset()
{
	_img_block_w = 0; //server side image width in pixel
	_img_block_h = 0; //server side image height in pixel
	_num_block_x = 0;
	_num_block_y = 0;
	_num_layer = 0;
	_pixel_bu = 0;
	x0 = 0;
	y0 = 0;
}

int PrjConst::img_block_w() const
{
	return _img_block_w;
}

int PrjConst::img_block_h() const
{
	return _img_block_h;
}

int PrjConst::num_block_x() const
{
	return _num_block_x;
}

int PrjConst::num_block_y() const
{
	return _num_block_y;
}

int PrjConst::num_layer() const
{
	return _num_layer;
}

int PrjConst::max_scale() const
{
	return _max_scale;
}

double PrjConst::pixel_bu() const
{
	return _pixel_bu;
}

int PrjConst::left_bound() const
{
	return x0;
}

int PrjConst::right_bound() const
{
	return x0 + tot_width_bu() - 1;
}

int PrjConst::top_bound() const
{
	return y0;
}

int PrjConst::bottom_bound() const
{
	return y0 + tot_height_bu() - 1;
}

int PrjConst::tot_width_bu() const
{
	return _img_block_w * _num_block_x * _pixel_bu;
}

int PrjConst::tot_height_bu() const
{
	return _img_block_h * _num_block_y * _pixel_bu;
}

int PrjConst::tot_width_pixel() const
{
	return _img_block_w * _num_block_x;
}

int PrjConst::tot_height_pixel() const
{
	return _img_block_h * _num_block_y;
}

QRect PrjConst::bound_rect_bu() const
{
	return QRect(x0, y0, tot_width_bu(), tot_height_bu());
}

QRect PrjConst::bu2pixel(const QRect &r) const
{
	return QRect((r.left() - x0) / _pixel_bu, (r.top() - y0) / _pixel_bu,
		r.width() / _pixel_bu, r.height() / _pixel_bu);
}

QRect PrjConst::pixel2bu(const QRect &r) const
{
	return QRect(r.left() * _pixel_bu + x0, r.top() * _pixel_bu + y0,
		r.width() * _pixel_bu, r.height() * _pixel_bu);
}

QPoint PrjConst::bu2pixel(const QPoint &r) const
{
	return QPoint((r.x() - x0) / _pixel_bu, (r.y() - y0) / _pixel_bu);
}

QPoint PrjConst::pixel2bu(const QPoint &r) const
{
	return QPoint(r.x() *_pixel_bu + x0, r.y() * _pixel_bu + y0);
}

QSize PrjConst::bu2pixel(const QSize &r) const
{
	return QSize(r.width() / _pixel_bu, r.height() / _pixel_bu);
}

QSize PrjConst::pixel2bu(const QSize &r) const
{
	return QSize(r.width() * _pixel_bu, r.height() * _pixel_bu);
}

struct EncodeImg {
	vector<uchar> buff;
	MapID id;
	int x0, y0;
};

struct DecodeImg {
	QImage img;
	MapID id;
	int x0, y0;
};

//It is running in global thread-pool
DecodeImg thread_decode_image(QSharedPointer<EncodeImg> pb)
{
	DecodeImg ret;
	ret.img.loadFromData(&(pb->buff[0]), pb->buff.size());
	ret.id = pb->id;
	ret.x0 = pb->x0;
	ret.y0 = pb->y0;
	return ret;
}

void destroy_thread(QObject * thread)
{
	qInfo("Thread %p is deleted", thread);
}

bool RenderImage::inited = false;

PrjConst * RenderImage::register_new_window(QObject * pobj)
{
	if (!inited) {
		inited = true;
		qRegisterMetaType<RenderType>("RenderType");
		qRegisterMetaType<string>("string");
	}
	QThread * render_thread = new QThread;
	RenderImage * render_image = new RenderImage;

	//Connect bkimg request
    if (!connect(pobj, SIGNAL(render_bkimg(string, string, const unsigned char, const QRect, const QSize, RenderType, const void *, bool)),
        render_image, SLOT(render_bkimg(string, string, const unsigned char, const QRect, const QSize, RenderType, const void *, bool))))
		qFatal("Connect render_bkimg failed");

    if (!connect(pobj, SIGNAL(render_bkimg_blocking(string, string, unsigned char, QRect, QSize, RenderType,QRect&, QImage &)),
        render_image, SLOT(render_bkimg_blocking(string, string, unsigned char, QRect, QSize,RenderType,QRect&,QImage &)), Qt::BlockingQueuedConnection))
        qFatal("Connect render_bkimg_blocking failed");

    if (!connect(render_image, SIGNAL(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const void *)),
		pobj, SLOT(render_bkimg_done(const unsigned char, const QRect, const QSize, QImage, bool, const void *)))) 
		qFatal("Connect render_bkimg_done failed");
	
	//Widget destroyed cause render_image delete
	if (!connect(pobj, SIGNAL(destroyed()), render_image, SLOT(deleteLater())))
		qFatal("Connect view destroyed failed");
	/*render_image delete cause render_thread quit
	if (!connect(render_image, SIGNAL(destroyed()), render_thread, SLOT(quit())))
		qFatal("Connect render_image destroyed failed");*/
	//render_thread quit cause render_thread delete
	if (!connect(render_thread, SIGNAL(finished()), render_thread, SLOT(deleteLater())))
		qFatal("Connect thread finished failed");
	//render_thread delete is printed by destroy_thread
	if (!connect(render_thread, &QObject::destroyed, destroy_thread))
		qFatal("connect thread destroy failed");

	qInfo("RenderImage %p and Thread %p are created by View %p", render_image, render_thread, pobj);
    render_image->moveToThread(render_thread);
	render_thread->start();
	return &(render_image->prj_cnst);
}

RenderImage::RenderImage(QObject *parent) : QObject(parent)
{
}

RenderImage::~RenderImage()
{
	qInfo("RenderImage %p is deleted", this);
	this->thread()->quit();
}
/*
 * rect's unit is bu
 * screen's unit is pixel
 */
void RenderImage::render_bkimg(string prj, string license, const unsigned char layer, const QRect rect,
	const QSize screen, RenderType rt, const void * view, bool preload_enable)
{
	//check if prj is new, if yes, open prj
	if (bk_img.isNull()) {
        bk_img = bkimg_faty.open(prj, license, CACHE_SIZE);
        if (bk_img.isNull()) {
            qCritical("open prj %s fail!", prj.c_str());
			return;
        }
		int width, bx, by;
		width = bk_img->getBlockWidth();
		bk_img->getBlockNum(bx, by);
        if (width == 0 || bx == 0 || by == 0)  {
            qCritical("get prj %s width and block fail!", prj.c_str());
            return;
        }
		prj_cnst.set(width, width, bx, by, bk_img->getLayerNum(), bk_img->getMaxScale());
	}
	if (bk_img->get_prj_name() != prj) {
		prj_cnst.reset();
		bk_img->adjust_cache_size(-CACHE_SIZE);
        bk_img = bkimg_faty.open(prj, license, CACHE_SIZE);
        if (bk_img.isNull()) {
            qCritical("reset and open prj %s fail!", prj.c_str());
			return;
        }
		preimg_map.clear();
		int width, bx, by;
		width = bk_img->getBlockWidth();
		bk_img->getBlockNum(bx, by);
        if (width == 0 || bx == 0 || by == 0)  {
            qCritical("get prj %s width and block fail!", prj.c_str());
            return;
        }
		prj_cnst.set(width, width, bx, by, bk_img->getLayerNum(), bk_img->getMaxScale());
	}

	//Following code decide scale
	unsigned char scale;
	QRect rpixel = prj_cnst.bu2pixel(rect & prj_cnst.bound_rect_bu());
	int w = prj_cnst.img_block_w(), h = prj_cnst.img_block_h();

	for (scale = 0; scale <= prj_cnst.max_scale(); scale++) {
		if (rpixel.width() <= 2 * w  && rpixel.height() <= 2 * h)
			break;
		w = w << 1;
		h = h << 1;
	}
	if (scale > prj_cnst.max_scale()) {
		qWarning("Repair me, still not do"); //TODO
		return;
	}
	Q_ASSERT(rpixel.right() / w - rpixel.left() / w <= 2 && rpixel.bottom() / h - rpixel.top() / h <= 2);
	/*Following code make sure image size = 3*prj_cnst.img_block_w(), it modify rpixel, 
	if don't like, delete following code
	*/
	while (rpixel.right() / w - rpixel.left() / w <= 1 && rpixel.width() >= rpixel.height()) {
		if (rpixel.right() + w < prj_cnst.tot_width_pixel())
			rpixel.adjust(0, 0, w, 0);
		else
			rpixel.adjust(-w, 0, 0, 0);
	}
	while (rpixel.bottom() / h - rpixel.top() / h <= 1 && rpixel.width() < rpixel.height()) {
		if (rpixel.bottom() + h < prj_cnst.tot_height_pixel())
			rpixel.adjust(0, 0, 0, h);
		else
			rpixel.adjust(0, -h, 0, 0);
	}	
	Q_ASSERT(rpixel.right() / w - rpixel.left() / w == 2 && rpixel.width() >= rpixel.height() || 
		rpixel.bottom() / h - rpixel.top() / h == 2 && rpixel.width() < rpixel.height());
	//Following code generate QImage 
	QImage image((rpixel.right() / w - rpixel.left() / w + 1) *prj_cnst.img_block_w(),
		(rpixel.bottom() / h - rpixel.top() / h + 1) *prj_cnst.img_block_h(),
		QImage::Format_RGB32);
	image.fill(QColor(160, 160, 160));
	QPainter painter(&image);
    vector<QFuture<DecodeImg> > subimgs;
	map <MapID, unsigned int> curimg_map;
    qDebug("Renderimg, l=%d, s=%d, w=(%d,%d), Rp=(%d,%d,%d,%d) ", layer, scale, w, h, rpixel.left() / w, rpixel.top() / h, rpixel.right() / w, rpixel.bottom() / h);
	for (int x = rpixel.left() / w, x0 = 0; x <= rpixel.right() / w; x++, x0 += prj_cnst.img_block_w())
		for (int y = rpixel.top() / h, y0 = 0; y <= rpixel.bottom() / h; y++, y0 += prj_cnst.img_block_h()) 
			if (x >= 0 && x < prj_cnst.num_block_x() && y >= 0 && y < prj_cnst.num_block_y()) {
			MapID id = sxy2mapid(layer, scale, x << scale, y << scale);
			list<PrevImg>::iterator iter;

			map<MapID, unsigned int>::iterator preimg_it = preimg_map.find(id);
			if (preimg_it != preimg_map.end() && rt != NO_NEED_RETURN) {
				unsigned x1 = preimg_it->second & 0xffff;
				unsigned y1 = preimg_it->second >> 16;
				painter.drawImage(QRect(x0, y0, prj_cnst.img_block_w(), prj_cnst.img_block_h()), prev_img,
					QRect(x1, y1, prj_cnst.img_block_w(), prj_cnst.img_block_h()));			
				curimg_map[id] = (y0 << 16) | x0;
				qDebug("draw from (%d,%d) to (%d,%d)", y1, x1, y0, x0);
			} else { //Not in prev_img cache
				EncodeImg * pb = new EncodeImg;
				int ret = bk_img->getRawImgByIdx(pb->buff, layer, x << scale, y << scale, scale, 0);
                //Q_ASSERT(ret == 0 && !pb->buff.empty());
                if (ret!=0 || pb->buff.empty()) {
                    qWarning("load image fail, x=%d,y=%d,scale=%d", x<< scale, y<<scale, scale);
                    QImage black_img(prj_cnst.img_block_w(), prj_cnst.img_block_h(), QImage::Format_RGB32);
                    black_img.fill(QColor(0, 0, 0));
                    painter.drawImage(QRect(x0, y0, prj_cnst.img_block_w(), prj_cnst.img_block_h()), black_img);
                }
                else
				if (rt != NO_NEED_RETURN) {
					pb->x0 = x0;
					pb->y0 = y0;
					pb->id = id;
					qDebug("decode image (%d,%d) to (%d,%d)", y, x, y0, x0);
					subimgs.push_back(QtConcurrent::run(thread_decode_image, QSharedPointer<EncodeImg>(pb)));
					curimg_map[id] = (y0 << 16) | x0;
				}
				else
					delete pb;
			}
		}

	if (rt != NO_NEED_RETURN) {
		for (int i = 0; i<(int)subimgs.size(); i++) {
			DecodeImg subimg = subimgs[i].result();
			qDebug("draw decode image (%d,%d)", subimg.y0, subimg.x0);
			painter.drawImage(QRect(subimg.x0, subimg.y0, prj_cnst.img_block_w(), prj_cnst.img_block_h()), subimg.img);			
		}
		subimgs.clear();
		QRect render_rect_pixel(QPoint(rpixel.left() / w * w, rpixel.top() / h * h),
			QPoint(rpixel.right() / w * w + w - 1, rpixel.bottom() / h * h + h - 1));
		QRect render_rect = prj_cnst.pixel2bu(render_rect_pixel);
        switch (rt) {
        case PRINT_SCREEN_NO_RETURN: {
            QRect source((double)image.width() * (rect.left() - render_rect.left()) / render_rect.width(),
                (double)image.height()* (rect.top() - render_rect.top()) / render_rect.height(),
                (double)image.width() * rect.width() / render_rect.width(),
                (double)image.height()* rect.height() / render_rect.height());
            QImage match_img = image.copy(source);
            char s[50];
            sprintf(s, "sc_%d_%d_M", rpixel.top() / h, rpixel.left() / w);
            QString filename(s);
            filename.append('0' + layer);
            filename.append(".jpg");
            match_img.save(filename, "JPG");
            return;
            }
        case RETURN_EXACT_MATCH: {
            QRect source((double)image.width() * (rect.left() - render_rect.left()) / render_rect.width(),
                (double)image.height()* (rect.top() - render_rect.top()) / render_rect.height(),
                (double)image.width() * rect.width() / render_rect.width(),
                (double)image.height()* rect.height() / render_rect.height());
            QImage match_img = image.copy(source).scaled(screen);
            emit render_bkimg_done(layer, rect, screen, match_img, true, view);
            return;
            }
        case RETURN_WHEN_PART_READY:
        case RETURN_UNTIL_ALL_READY: {
			preimg_map = curimg_map;
            prev_img = image;
            pre_render_rect = render_rect;
            if (view != this)
                emit render_bkimg_done(layer, render_rect, screen, image, true, view);
            return;
            }
        }
	}
}

void RenderImage::render_bkimg_blocking(string prj, string license, unsigned char layer, QRect rect,
     QSize screen, RenderType rt, QRect & render_rect, QImage & img)
{   
    QRect request = prj_cnst.pixel2bu(rect);
    render_bkimg(prj, license, layer, request, screen, RETURN_UNTIL_ALL_READY, this, false);

    if (rt == RETURN_EXACT_MATCH) {
        QRect source((double)prev_img.width() * (request.left() - pre_render_rect.left()) / pre_render_rect.width(),
            (double)prev_img.height()* (request.top() - pre_render_rect.top()) / pre_render_rect.height(),
            (double)prev_img.width() * request.width() / pre_render_rect.width(),
            (double)prev_img.height()* request.height() / pre_render_rect.height());
        render_rect = rect;
        qDebug("render_bkimg_blocking, reuqest(x=%d,y=%d,w=%d, h=%d), render(x=%d,y=%d,w=%d,h=%d)",
               request.left(), request.top(), request.width(), request.height(),
               pre_render_rect.left(), pre_render_rect.top(), pre_render_rect.width(), pre_render_rect.height());
        img = prev_img.copy(source).scaled(screen);
    }
    else {
        img = prev_img;
        render_rect = prj_cnst.bu2pixel(pre_render_rect);
    }

    qDebug("render_bkimg_blocking, render(x=%d,y=%d, w=%d, h=%d), img(w=%d,h=%d)",
           render_rect.left(), render_rect.top(), render_rect.width(), render_rect.height(), img.width(), img.height());
    return;
}
