#include "iclayer.h"
#include <fstream>
//#include "Debug.h"
#include <QPixmap>
typedef long long int64;
using namespace std;

class ICLayer
{
    vector<vector<int64> > bias;
    vector<vector<int> > len;
    vector<Point> corners;
    string file_;
    ifstream fin;
public:
    ICLayer(string& file);
    ~ICLayer() {fin.close();}
    int readLayerFile(const string& file);
    int getCorners(vector<Point > & corners);
    int getBlockWidth();
    int getRawImgByIdx(vector<uchar> & buff, int idx, int ovr, unsigned reserved);
};


int ICLayer::readLayerFile(const string& file)
{
    //DEBUG_AUTO_GUARD;
    bias.clear();
    corners.clear();
    //DEBUG_LOG(QString::fromStdString(file));
    fin.open(file.c_str(), ios::binary);
    if (!fin.is_open())return -1;

    int ovr_num = 5;
    int64 block_bias = 0;
    int sizeofint = sizeof(int);
    int  head_sz = (ovr_num + 4)*sizeof(int);

    fin.seekg(0, ios::end);
    int64 file_len = fin.tellg();
    fin.seekg(0, ios::beg);

    while (block_bias < file_len)
    {
        fin.seekg(block_bias, ios::beg);
        int block_len;
        fin.read((char*)&block_len, sizeofint);


        vector<int64> bias_sub;//x=bias y=length;
        vector<int> len_sub;
        int bias_in = 0;
        for (int i = 0; i < ovr_num + 1; i++)
        {
            int64 bias_tmp;
            int len_tmp;
            fin.read((char*)&len_tmp, sizeofint);
            bias_tmp = block_bias + head_sz + bias_in;
            //cout << "bias:" << pt.x << "   len:" << pt.y << endl;
            bias_sub.push_back(bias_tmp);
            len_sub.push_back(len_tmp);
            bias_in += len_tmp;
        }
        Point corner;
        fin.read((char*)&corner.x, sizeofint);
        fin.read((char*)&corner.y, sizeofint);
        corners.push_back(corner);

        bias.push_back(bias_sub);
        len.push_back(len_sub);
        block_bias += block_len;
    }
    return 0;
}

ICLayer::ICLayer(string& file) :file_(file)
{
   // CDebug::init(true);
    if (readLayerFile(file)!=0)
        qFatal("layer file not exist");
}

int ICLayer::getCorners(vector< ::Point >& corners)
{
    //DEBUG_AUTO_GUARD;
    corners.clear();
    corners = this->corners;
    return 0;
}

int ICLayer::getBlockWidth()
{
    return 1024;
}
/*
int ICLayer::getImgByIdx(QImage& qimage, int idx, int ovr)
{
    //DEBUG_AUTO_GUARD;
    if (idx > corners.size())return -3;
    if (ovr > 5)return -2;
    return readLayerBlock(file_.c_str(), bias[idx][ovr], len[idx][ovr], qimage);
}*/

int ICLayer::getRawImgByIdx(vector<uchar> & buff, int idx, int ovr, unsigned reserved)
{
    if (idx > corners.size()) return -3;
    if (ovr > 5)return -2;
    buff.resize(len[idx][ovr] + reserved);
    fin.seekg(bias[idx][ovr], ios::beg);
    fin.read((char*)&buff[reserved], len[idx][ovr]);
    return 0;
}

ICLayerWr::ICLayerWr()
{
    layer_ = NULL;
}

ICLayerWr::ICLayerWr(string file)
{
    layer_ = new ICLayer(file);
}
ICLayerWr::~ICLayerWr()
{
    if (layer_)delete(layer_);
}
int ICLayerWr::getCorners(vector<Point>& corners)
{
    layer_->getCorners(corners);
    return 0;
}

void ICLayerWr::setFile(string file)
{
    if (layer_)
        delete(layer_);
    layer_ = new ICLayer(file);
}

int ICLayerWr::getBlockWidth()
{
    return layer_->getBlockWidth();
}
int ICLayerWr::getRawImgByIdx(vector<uchar> & buff, int idx, int ovr, unsigned reserved)
{
    return layer_->getRawImgByIdx(buff, idx, ovr, reserved);
}
