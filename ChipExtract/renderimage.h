#ifndef RENDERIMAGE_H
#define RENDERIMAGE_H
#include "iclayer.h"
#include <QImage>
#include <QRect>
#include <list>
#include <map>
#include <QThread>
using namespace std;

typedef unsigned long long MapID;
#define INVALID_MAP_ID 0xffffffffffffff

typedef enum {
    RETURN_UNTIL_ALL_READY,  //Return image when all clear subimages are ready, request may be sent to server and wait all clear subimage ready, caller get only one return, and may wait several seconds.
    RETURN_WHEN_PART_READY,  //Return image immediately, if clear subimage is not ready use blur image instead, and then request may be sent to server and wait all subimage ready, and then return more clear image to caller. So caller may get two or more return. One return is immediate blur image, Another return is clear image after several seconds.
	RETURN_EXACT_MATCH,
    NO_NEED_RETURN			 //Caller don't need return, request may be sent to server, so caller can use this to preload, next time when caller call RETURN_UNTIL_ALL_READY or RETURN_WHEN_PART_READY, reply time is less
} RenderType;

class PrjConst
{	
public:
	friend class RenderImage;
	PrjConst();
	int img_block_w() const; //server side image width in pixel
	int img_block_h() const; //server side image height in pixel
	int num_block_x() const; //server side image num in row
	int num_block_y() const; //server side image num in column
	int num_layer() const;
	int max_scale() const;
	double pixel_bu() const;
	int left_bound() const; //return in bu
	int right_bound() const; //return in bu
	int top_bound() const; //return in bu
	int bottom_bound() const; //return in bu
	int tot_width_bu() const; //return in bu
	int tot_height_bu() const; //return in bu
	int tot_width_pixel() const; //return in bu
	int tot_height_pixel() const; //return in bu
	QRect bound_rect_bu() const; //return bound rect
	QRect bu2pixel(const QRect &r) const; //r in bu, return in pixel
	QRect pixel2bu(const QRect &r) const; //r in pixelm return in bu
	QPoint bu2pixel(const QPoint &r) const; //r in bu, return in pixel
	QPoint pixel2bu(const QPoint &r) const; //r in pixelm return in bu
	QSize bu2pixel(const QSize &r) const; //r in bu, return in pixel
	QSize pixel2bu(const QSize &r) const; //r in pixelm return in bu

protected:
	int _img_block_w;
	int _img_block_h;
	int _num_block_x;
	int _num_block_y;
	int _num_layer;
	int _max_scale;
	int x0, y0;
	double _pixel_bu; // 1 pixel length / 1 basic unit
	void set(int img_block_w, int img_block_h, int num_block_x, int num_block_y, int num_layer, int max_scale);
	void reset();
};

class RenderImage : public QObject
{
    Q_OBJECT
public:
    explicit RenderImage(QObject *parent = 0);
	~RenderImage();	
	static PrjConst * register_new_window(QObject * pobj);

signals:
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage image, bool finish, const void * view);

public slots:
    void render_bkimg(string prj, const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const void * view, bool preload_enable);

protected:
	static bool inited;
	static MapID sxy2mapid(unsigned char layer, unsigned char scale, unsigned short x, unsigned short y) {
		MapID ret;
		ret = layer;
		ret = ret << 8;
		ret |= scale;
		ret = ret << 16;
		ret |= x;
		ret = ret << 16;
		ret |= y;
		return ret;
	}

	static void mapid2sxy(MapID m, unsigned char &layer, unsigned char & scale, unsigned short & x, unsigned short &y)
	{
		layer = (m >> 40) & 0xff;
		scale = (m >> 32) & 0xff;
		x = (m >> 16) & 0xffff;
		y = m & 0xffff;
	}

protected:
	struct PrevImg {
		MapID id;
		QImage img;
		PrevImg(MapID _id, QImage _img) {
			id = _id;
			img = _img;
		}
	};
	QSharedPointer<BkImgInterface> bk_img;
	QImage prev_img;
	map <MapID, unsigned int> preimg_map;
	PrjConst prj_cnst;
};

#endif // RENDERIMAGE_H
