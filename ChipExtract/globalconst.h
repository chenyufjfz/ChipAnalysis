#ifndef GLOBALCONST_H
#define GLOBALCONST_H
#include <QRect>
#include <QPoint>
class GlobalConst
{
public:
    GlobalConst();
    int img_block_w() const; //server side image width in pixel
    int img_block_h() const; //server side image height in pixel
    int num_block_x() const; //server side image num in row
    int num_block_y() const; //server side image num in column
    int num_layer() const;
    double pixel_bu() const;
    int left_bound() const; //return in bu
    int right_bound() const; //return in bu
    int top_bound() const; //return in bu
    int bottom_bound() const; //return in bu
    int tot_width_bu() const; //return in bu
    int tot_height_bu() const; //return in bu
	QRect bound_rect_bu() const; //return bound rect
    QRect bu2pixel(const QRect &r) const; //r in bu, return in pixel
    QRect pixel2bu(const QRect &r) const; //r in pixelm return in bu
    QPoint bu2pixel(const QPoint &r) const; //r in bu, return in pixel
    QPoint pixel2bu(const QPoint &r) const; //r in pixelm return in bu
    QSize bu2pixel(const QSize &r) const; //r in bu, return in pixel
    QSize pixel2bu(const QSize &r) const; //r in pixelm return in bu
    void set(int img_block_w, int img_block_h, int num_block_x, int num_block_y, int num_layer);

protected:
    int _img_block_w;
    int _img_block_h;
    int _num_block_x;
    int _num_block_y;
    int _num_layer;
    int x0, y0;
    double _pixel_bu; // 1 pixel length / 1 basic unit
};

#endif // GLOBALCONST_H
