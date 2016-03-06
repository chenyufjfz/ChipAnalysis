#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include <QThread>
#include "RakPeerInterface.h"

class ServerThread : public QThread
{
    Q_OBJECT

public:
    ServerThread();
    void end();

protected:
    bool finish;
    int server_port;
    int max_user;
    void run() Q_DECL_OVERRIDE;
};

#endif // SERVERTHREAD_H
