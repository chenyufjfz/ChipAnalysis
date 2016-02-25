#ifndef CONNECTVIEW_H
#define CONNECTVIEW_H

#include <QWidget>
#include <QKeyEvent>
#include "renderimage.h"

class ConnectView : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectView(QWidget *parent = 0);

signals:
    void render_bkimg(const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const QObject * view, bool preload_enable);

public slots:
    void server_connected();
    void server_disconnected();
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage & image, bool finish, const QObject * view);

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void adjust_viewrect();

protected:
    bool connect_to_server;
    QImage render_img;
    QRect render_rect, view_rect;
    QPoint center;
    double scale;
    unsigned char bk_layer, render_bk_layer;
};

#endif // CONNECTVIEW_H
