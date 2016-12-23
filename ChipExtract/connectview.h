#ifndef CONNECTVIEW_H
#define CONNECTVIEW_H

#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include "renderimage.h"
#include "searchobject.h"

class ConnectView : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectView(QWidget *parent = 0);


signals:
    void render_bkimg(const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const QObject * view, bool preload_enable);
    void train_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
    void extract_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);
    void extract_wire_via(QSharedPointer<VWSearchRequest> preq, const QRect rect);
    void mouse_change(QPoint pos, QString msg);

public slots:
    void server_connected();
    void server_disconnected();
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage image, bool finish, const QObject * view);
    void extract_cell_done(QSharedPointer<SearchResults> prst);
    void extract_wire_via_done(QSharedPointer<SearchResults> prst);

public:
    void train(bool cell_train, int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void extract(bool cell_train, int i1, int i2, int i3, int i4, float f1, float f2, float f3);
    void load_objects(string file_name);

protected:
	void draw_element(QPainter &painter);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);    
    void mouseMoveEvent(QMouseEvent *event);

protected:
    bool connect_to_server;
	QImage render_img;
    QRect render_rect, view_rect;
    QPoint center;
    double scale;
    unsigned char bk_layer, render_bk_layer;
};

#endif // CONNECTVIEW_H
