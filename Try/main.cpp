#include "iclayer.h"
#include <iostream>
#include <QRect>

using namespace std;
int test_raknet();
int test_lmdb();
void test_iclayer()
{
    QString str = "../../M6.dat";
    ICLayerWr layer(str.toStdString());
    cout << "width:" << layer.getBlockWidth() << endl;

    std::vector<Point> corners;
    layer.getCorners(corners);
    cout << "nums:" << corners.size() << endl;
    for (int i = 0; i < corners.size(); i++)
    {
        cout << "x:" << corners[i].x << "," << corners[i].y<<endl;
    }
    QImage qimage;
    layer.getImgByIdx(qimage, 0, 0);
    qimage.save("../../a00.jpg");
}

int main()
{
    //test_lmdb();
    //test_raknet();
    QRect rect(1, 1, 10, 10);
    rect.moveLeft(0);
    printf("l=%d, r=%d, t=%d, b=%d\n", rect.left(), rect.right(), rect.top(), rect.bottom());
    rect.moveTop(0);
    printf("l=%d, r=%d, t=%d, b=%d\n", rect.left(), rect.right(), rect.top(), rect.bottom());
    rect.moveRight(10);
    printf("l=%d, r=%d, t=%d, b=%d\n", rect.left(), rect.right(), rect.top(), rect.bottom());
    rect.moveBottom(10);
    printf("l=%d, r=%d, t=%d, b=%d\n", rect.left(), rect.right(), rect.top(), rect.bottom());
    rect.adjust(-1,-1,2,2);
    printf("l=%d, r=%d, t=%d, b=%d\n", rect.left(), rect.right(), rect.top(), rect.bottom());
    //test_iclayer();

}
