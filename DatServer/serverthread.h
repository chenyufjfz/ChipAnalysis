#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include <QThread>
#include "serverperclient.h"
#include <map>

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
    BkImgDB imgdb;
    map<unsigned long long, ServerPerClient*> server_pools;
    void run() Q_DECL_OVERRIDE;
};

#endif // SERVERTHREAD_H
