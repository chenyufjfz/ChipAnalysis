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
#include "vwextract.h"
using namespace std;

class CellExtractService : public QObject {
    Q_OBJECT
protected:
    CellExtract * ce[64];
	unsigned token;

public:
	CellExtractService(unsigned _token, QObject *parent = 0);
	~CellExtractService();

public slots:
	void cell_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> packet);
};

class VWExtractService : public QObject {
    Q_OBJECT
protected:
    VWExtract * vwe;
	unsigned token;

public:
	VWExtractService(unsigned _token, QObject *parent = 0);
	~VWExtractService();

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
