#ifndef ELEMENTCLIENT_H
#define ELEMENTCLIENT_H

#include <QObject>

class ElementClient : public QObject
{
    Q_OBJECT
public:
    explicit ElementClient(QObject *parent = 0);

signals:

public slots:
};

#endif // ELEMENTCLIENT_H
