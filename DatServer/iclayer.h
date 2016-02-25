#ifndef ICLAYER_H
#define ICLAYER_H

#include <vector>
#include <QImage>


struct Point
{
    int x;
    int y;
};

class ICLayer;
class ICLayerWr
{
public:
    ICLayerWr();
    ICLayerWr(std::string file);
    ~ICLayerWr();
    int getCorners(std::vector<Point>& corners);
    int getBlockWidth();
    int getRawImgByIdx(std::vector<uchar> & buff, int idx, int ovr, unsigned reserved);
    void setFile(std::string file);
private:
    ICLayer * layer_;
};

#endif // ICLAYER_H
