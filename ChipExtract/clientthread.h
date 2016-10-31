#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include <QThread>
#include "RakPeerInterface.h"

enum ConnectState{
    NO_CONNECT,
    ACQUIRE_IMG_INFO,
    CONNECT
};

class ClientThread : public QThread
{
    Q_OBJECT

public:
    enum ConnectState connect_state;
    ClientThread();
    void end();

signals:
    void bkimg_packet_arrive(void * p);
    void search_packet_arrive(void* packet);
    void server_connected();
    void server_disconnected();
protected:
    bool finish;
    void run() Q_DECL_OVERRIDE;
};

#endif // CLIENTTHREAD_H
