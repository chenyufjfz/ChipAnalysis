#ifndef SERVERPERCLIENT_H
#define SERVERPERCLIENT_H
#include "iclayer.h"
#include <QObject>
#include <vector>
#include <string>
#include <QThread>
#include <map>
#include "RakPeerInterface.h"
#include "cellextract.h"
#include "vwextract_public.h"

using namespace std;

class CellExtractService : public QObject {
    Q_OBJECT
protected:
	unsigned token;
	RakNet::SystemAddress cli_addr;

public:
	CellExtractService(unsigned _token, QObject *parent = 0);
	~CellExtractService();
	void notify(MarkObj * o);

public slots:
	void cell_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> packet);
};

class VWExtractService : public QObject {
    Q_OBJECT
protected:
    VWExtract * vwe_single;
	unsigned token;
	unsigned req_command;
	RakNet::SystemAddress cli_addr;

public:
	VWExtractService(unsigned _token, QObject *parent = 0);
	~VWExtractService();
	void notify(MarkObj * o);
public slots:
	void vw_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> packet);
};

Q_DECLARE_METATYPE(QSharedPointer<RakNet::Packet>)
Q_DECLARE_METATYPE(QSharedPointer<BkImgInterface>)

class ServerPerClient : public QObject
{
    Q_OBJECT
protected:
	static bool inited;

protected:
    RakNet::SystemAddress cli_addr;
    QThread * work_thread;
	struct ServicePerToken {
		QSharedPointer<BkImgInterface> bk_img;
		CellExtractService * ce_service;
		VWExtractService * vw_service;
	};
	map<unsigned, ServicePerToken> service_pool;

public:
	explicit ServerPerClient(RakNet::SystemAddress cli_addr_, QObject *parent = 0);
    ~ServerPerClient();

signals:
	void cell_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> p);
	void vw_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> p);

public:
	void handle_client_req(QSharedPointer<RakNet::Packet> p);
};

#endif // SERVERPERCLIENT_H
