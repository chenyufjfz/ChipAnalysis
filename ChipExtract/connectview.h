#ifndef CONNECTVIEW_H
#define CONNECTVIEW_H

#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include "renderimage.h"
#include "searchobject.h"
#include "objectdb.h"

#define SEARCH_PARALLEL     1
#define SEARCH_POLYGEN      2
#define SEARCH_WIRE_ONLY    4
#define SEARCH_VIA_ONLY     8

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
    void render_bkimg(string prj, string _license, const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const void * view, bool preload_enable);
    void render_bkimg_blocking(string prj, string _license,unsigned char layer, QRect rect_pixel,
                      QSize screen, RenderType rt, QRect & render_pixel, QImage & img);
    void train_cell(string prj, string license, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
	void extract_cell(string prj, string license, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);
	void extract_wire_via(string prj, string license, QSharedPointer<VWSearchRequest> preq, const QRect rect, int option);
	void extract_single_wire(string prj, string license, int layer, int wmin, int wmax, int ihigh, int opt,
                    int gray_th, int channel, int scale, void * loc, float cr, float cg, int shape_mask);
	void train_via_ml(string prj, string license, int layer, int label, int dmin, int dmax, int x, int y, int param3, int param4, int param5, int param6);
	void train_wire_ml(string prj, string license, int layer, int label, int wmin_x, int wmin_y, int via_d, int x, int y, int param3);
	void get_train_vw_ml(string prj, string license, int layer);
	void del_vw_ml(string prj, string license, int layer, int d, int x, int y);
	void extract_ml(string prj, string license, int layer_min, int layer_max, QPolygon area, int wmin_x, int wmin_y, int via_d, int param3, int param4, int opt);
    void mouse_change(QPoint pos, QString msg);
	void notify_progress(float);

public slots:
	void server_connected();
	void server_disconnected();
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage image, bool finish, const void * view);
    void extract_cell_done(QSharedPointer<SearchResults> prst);
    void extract_wire_via_done(QSharedPointer<SearchResults> prst);
    void extract_single_wire_done(QSharedPointer<SearchResults> prst);
	void train_vw_ml_done(QSharedPointer<SearchResults>);
	void del_vw_ml_done();
	void get_train_vw_ml_done(QSharedPointer<SearchResults> prst);
	void extract_ml_done(QSharedPointer<SearchResults>);

public:
    void cell_train(int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void cell_extract(int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void wire_extract(VWSearchRequest & vp, int opt);
    void single_wire_extract(int layer, int wmin, int wmax, int opt,
		int gray_th, int channel, vector<QPoint> org, float cr = -1, float cg = -1, int shape_mask = 0xff);
	void ml_via_train(int layer, int dmin, int dmax, int label, QPoint v);
	void ml_wire_train(int layer, int label, QPoint v);
	void ml_vw_del(int layer, int d, QPoint v);
	void get_ml_obj(int layer);
	void vwml_extract(int layer, int opt);
    void load_objects(string file_name);
    void set_mark(unsigned char type, unsigned char type2);
    void set_license(string _license);
    void clear_objs();
    void clear_wire_via();
	void set_prj_file(string _prj_file);
    void goto_xy(int x, int y, int layer);
	string get_prj_file();
	int get_current_layer();
	void get_dia(vector<int> & dia_);
	void set_dia(const vector<int> & dia_);
    void get_wide_xy(vector<int> & wide_x_, vector<int> & wide_y_, vector<int> & via_d_, vector<int> & force_, vector<int> & force_wire_);
    void set_wide_xy(const vector<int> & wide_x_, const vector<int> & wide_y_, vector<int> & via_d_, vector<int> & force_, vector<int> & force_wire_);

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
    vector<int> dia, wide_x, wide_y, via_d, force, force_wire;
    bool hide_element;
    MarkState ms;
	QImage render_img, render_img_2;
    QRect render_rect, view_rect, view_rect_2, screen_2;
    QPoint center, center_2;
    double scale, scale_2;
    unsigned char bk_layer, bk_layer_2, render_bk_layer;
	PrjConst *pcst;
    string prj_file, license;
};

#endif // CONNECTVIEW_H
