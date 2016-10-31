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

class SearchObject : public QObject
{
    Q_OBJECT
public:
    bool connect_to_server;
    explicit SearchObject(QObject *parent = 0);

signals:
    void extract_cell_done(QSharedPointer<SearchResults>);

public slots:
    void server_connected();
    void server_disconnected();
    void search_packet_arrive(void * p);
    void train_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
    void extract_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);

};

#endif // SEARCHOBJECT_H
