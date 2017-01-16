#ifndef SEARCHOBJECT_H
#define SEARCHOBJECT_H

#include <QObject>
#include <vector>
#include <QtCore/QMetaType>
#include <QtCore/QSharedPointer>
#include <QRect>

#include "communication.hpp"
#include "markobj.h"
using namespace std;

struct SearchResults {
    vector<MarkObj> objs;
};
Q_DECLARE_METATYPE(QSharedPointer<SearchResults>)

struct SearchRects {
    vector<unsigned char> dir;
    vector<QRect> rects;
};
Q_DECLARE_METATYPE(QSharedPointer<SearchRects>)

struct LayerParam {
    int layer; //which layer
    int wire_wd; //wire width
    int via_rd; //via radius
    int grid_wd; //grid width
    float param1; //via th, close to 1, higher threshold
    float param2; //wire th, close to 1, higher threshold
    float param3; //via_cred vs wire_cred, if via_cred> wire_cred, beta>1; else <1
    float param4;
    unsigned long long rule; //rule affect bbfm
    unsigned long long warn_rule; //warnrule affect report
    LayerParam(int _layer, int _wire_wd, int _via_rd, unsigned long long _rule, unsigned long long _warn_rule,
               int _grid_wd, float _param1, float _param2, float _param3, float _param4) {
        layer = _layer;
        wire_wd = _wire_wd;
        via_rd = _via_rd;
        grid_wd = _grid_wd;
        rule = _rule;
        warn_rule = _warn_rule;
        param1 = _param1;
        param2 = _param2;
        param3 = _param3;
        param4 = _param4;
    }
};

struct VWSearchRequest {
    vector<LayerParam> lpa;
};
Q_DECLARE_METATYPE(QSharedPointer<VWSearchRequest>)

class SearchObject : public QObject
{
    Q_OBJECT
public:
    bool connect_to_server;
    explicit SearchObject(QObject *parent = 0);

signals:
    void extract_cell_done(QSharedPointer<SearchResults>);
    void extract_wire_via_done(QSharedPointer<SearchResults>);

public slots:
    void server_connected();
    void server_disconnected();
    void search_packet_arrive(void * p);
    void train_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
    void extract_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);
    void extract_wire_via(QSharedPointer<VWSearchRequest> preq, const QRect rect);
};

#endif // SEARCHOBJECT_H
