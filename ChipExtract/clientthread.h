#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include <QThread>
#include "RakPeerInterface.h"

class ClientThread : public QThread
{
    Q_OBJECT

public:
    ClientThread();
    void end();

signals:
    void bkimg_packet_arrive(void * p);
    void server_connected();
    void server_disconnected();
protected:
    bool finish;
    void run() Q_DECL_OVERRIDE;
};

#endif // CLIENTTHREAD_H
