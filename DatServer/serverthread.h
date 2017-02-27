#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include <QThread>
#include "serverperclient.h"
#include "GetTime.h"
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
    map<unsigned long long, ServerPerClient*> server_pools;
    void run() Q_DECL_OVERRIDE;
	RakNet::TimeMS last_printtime;
};

#endif // SERVERTHREAD_H
