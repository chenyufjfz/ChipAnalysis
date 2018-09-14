#ifndef CONNECTVIEW_H
#define CONNECTVIEW_H

#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include "renderimage.h"
#include "searchobject.h"
#include "objectdb.h"

enum {
    CREATE_NEW_OBJ,
    CHOOSE_ANO_POINT,
    SELECT_EXIST
};

typedef enum {
	DISPLAY1,
	DISPLAY12_OP2,
	DISPLAY12_OP1,
} DisplayState;

struct MarkState {
    int state;
    unsigned char type;
    unsigned char type2;
    ElementObj draw_obj;
};

class ConnectView : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectView(const char * prj_name = 0, QWidget *parent = 0);

signals:
	void render_bkimg(string prj, const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const void * view, bool preload_enable);
    void render_bkimg_blocking(string prj, unsigned char layer, QRect rect_pixel,
                      QSize screen, RenderType rt, QRect & render_pixel, QImage & img);
    void train_cell(string prj, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
	void extract_cell(string prj, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);
    void extract_wire_via(string prj, QSharedPointer<VWSearchRequest> preq, const QRect rect, int option);
    void extract_single_wire(string prj, int layer, int wmin, int wmax, int ihigh, int opt,
                    int gray_th, int channel, int scale, int x, int y, float cr, float cg);
    void mouse_change(QPoint pos, QString msg);

public slots:
	void server_connected();
	void server_disconnected();
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage image, bool finish, const void * view);
    void extract_cell_done(QSharedPointer<SearchResults> prst);
    void extract_wire_via_done(QSharedPointer<SearchResults> prst);
    void extract_single_wire_done(QSharedPointer<SearchResults> prst);

public:
    void cell_train(int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void cell_extract(int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void wire_extract(VWSearchRequest & vp, int opt);
    void single_wire_extract(int layer, int wmin, int wmax, int opt,
                             int gray_th, int channel, QPoint org, float cr = -1, float cg = -1);
    void load_objects(string file_name);
    void set_mark(unsigned char type, unsigned char type2);
    void clear_objs();
	void set_prj_file(string _prj_file);
    void goto_xy(int x, int y, int layer);
	string get_prj_file();

protected:
    void draw_obj(ElementObj & obj, QPainter &painter);
	void draw_element(QPainter &painter);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);    
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected:
	DisplayState ds;
    bool hide_element;
    MarkState ms;
	QImage render_img, render_img_2;
    QRect render_rect, view_rect, view_rect_2, screen_2;
    QPoint center, center_2;
    double scale, scale_2;
    unsigned char bk_layer, bk_layer_2, render_bk_layer;
	PrjConst *pcst;
	string prj_file;
};

#endif // CONNECTVIEW_H
