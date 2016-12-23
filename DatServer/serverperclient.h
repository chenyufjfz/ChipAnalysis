#ifndef SERVERPERCLIENT_H
#define SERVERPERCLIENT_H
#include "iclayer.h"
#include <QObject>
#include <vector>
#include <string>
#include <QThread>
#include "RakPeerInterface.h"
#include "cellextract.h"
#include "vwextract.h"
using namespace std;

class BkImgDB {
public:
    vector<ICLayerWr *> bk_img_layers;
    void add_new_layer(string file_name) {
        bk_img_layers.push_back(new ICLayerWr(file_name, true));
    }

    ICLayerWr * get_layer(int layer) {
        return bk_img_layers[layer];
    }

    int get_layer_num() {
        return (int) bk_img_layers.size();
    }

    ~BkImgDB() {
        for (unsigned i=0; i<bk_img_layers.size(); i++)
            delete bk_img_layers[i];
    }
};


class CellExtractService : public QObject {
    Q_OBJECT
protected:
    CellExtract * ce[64];

public:
    explicit CellExtractService(QObject *parent = 0);

public slots:
	void cell_extract_req(void * p_cli_addr, void * bk_img_, void * p);
};

class VWExtractService : public QObject {
    Q_OBJECT
protected:
    VWExtract * vwe;

public:
    VWExtractService(QObject *parent = 0);

public slots:
    void vw_extract_req(void * p_cli_addr, void * bk_img_, void * p);
};

class ServerPerClient : public QObject
{
    Q_OBJECT
protected:
    BkImgDB * bk_img;
    RakNet::SystemAddress cli_addr;
    QThread * ano_thread;
    CellExtractService * ce_service;
    VWExtractService * vw_service;

public:
    explicit ServerPerClient(QObject *parent = 0);
    ~ServerPerClient();
    void prepare(BkImgDB * bk_img_, RakNet::SystemAddress cli_addr_);

signals:
	void cell_extract_req(void * p_cli_addr, void * bk_img, void * p);
    void vw_extract_req(void * p_cli_addr, void * bk_img_, void * p);

protected:
    void req_bk_img(unsigned char layer, unsigned char scale,
        unsigned short x, unsigned short y, unsigned char priority);
public:
    void handle_client_req(void * p);
};

#endif // SERVERPERCLIENT_H
