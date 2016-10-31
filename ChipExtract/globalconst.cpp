#include "globalconst.h"

GlobalConst gcst;
GlobalConst::GlobalConst()
{
    _img_block_w = 1024; //server side image width in pixel
    _img_block_h = 1024; //server side image height in pixel
    _num_block_x = 11;
    _num_block_y = 11;
    _num_layer = 1;
	_pixel_bu = 32768 / _img_block_w;
    x0=0;
    y0=0;
}

void GlobalConst::set(int img_block_w, int img_block_h, int num_block_x, int num_block_y, int num_layer)
{
    _img_block_w = img_block_w; //server side image width in pixel
    _img_block_h = img_block_h; //server side image height in pixel
    _num_block_x = num_block_x;
    _num_block_y = num_block_y;
    _num_layer = num_layer;
    _pixel_bu = 32768 / _img_block_w;
}

int GlobalConst::img_block_w() const
{
    return _img_block_w;
}

int GlobalConst::img_block_h() const
{
    return _img_block_h;
}

int GlobalConst::num_block_x() const
{
    return _num_block_x;
}

int GlobalConst::num_block_y() const
{
    return _num_block_y;
}

int GlobalConst::num_layer() const
{
    return _num_layer;
}

double GlobalConst::pixel_bu() const
{
    return _pixel_bu;
}

int GlobalConst::left_bound() const
{
    return x0;
}

int GlobalConst::right_bound() const
{
    return x0 + tot_width_bu() -1;
}

int GlobalConst::top_bound() const
{
    return y0;
}

int GlobalConst::bottom_bound() const
{
    return y0 + tot_height_bu() -1;
}

int GlobalConst::tot_width_bu() const
{
    return _img_block_w * _num_block_x * _pixel_bu;
}

int GlobalConst::tot_height_bu() const
{
    return _img_block_h * _num_block_y * _pixel_bu;
}

QRect GlobalConst::bound_rect_bu() const
{
	return QRect(x0, y0, tot_width_bu(), tot_height_bu());
}

QRect GlobalConst::bu2pixel(const QRect &r) const
{
    return QRect((r.left() -x0) /_pixel_bu, (r.top() -y0)/_pixel_bu,
                 r.width() /_pixel_bu, r.height() / _pixel_bu);
}

QRect GlobalConst::pixel2bu(const QRect &r) const
{
    return QRect(r.left() * _pixel_bu +x0, r.top() * _pixel_bu +y0,
                 r.width() * _pixel_bu, r.height() * _pixel_bu);
}

QPoint GlobalConst::bu2pixel(const QPoint &r) const
{
    return QPoint((r.x() -x0) / _pixel_bu, (r.y() -y0) / _pixel_bu);
}

QPoint GlobalConst::pixel2bu(const QPoint &r) const
{
    return QPoint(r.x() *_pixel_bu +x0, r.y() * _pixel_bu +y0);
}

QSize GlobalConst::bu2pixel(const QSize &r) const
{
    return QSize(r.width() / _pixel_bu, r.height() / _pixel_bu);
}

QSize GlobalConst::pixel2bu(const QSize &r) const
{
    return QSize(r.width() * _pixel_bu, r.height() * _pixel_bu);
}
